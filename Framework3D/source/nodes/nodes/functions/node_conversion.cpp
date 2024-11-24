
#if USTC_CG_WITH_CUDA
#include <cuda.h>
#include <cuda_runtime_api.h>
#include "CUDAExternal.h"
#include "CUDASurface.cuh"
#endif


#include <d3d12.h>

#include "../render/resource_allocator_instance.hpp"


#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "Nodes/socket_type_aliases.hpp"
#include "Nodes/socket_types/render_socket_types.hpp"
#include "Nodes/socket_types/stage_socket_types.hpp"
#include "RCore/Backend.hpp"
#include "utils/CUDA/CUDAException.h"
#include "boost/python/numpy.hpp"
#include "func_node_base.h"
#include "nvrhi/utils.h"
#include "pxr/imaging/hd/types.h"

#if USTC_CG_WITH_TORCH
#include <torch/torch.h>
#endif

namespace USTC_CG::node_conversion {
static void node_declare_Int_to_Float(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Int>("int");
    b.add_output<decl::Float>("float");
}

namespace bpn = boost::python::numpy;

static void node_exec_Int_to_Float(ExeParams params)
{
    auto i = params.get_input<int>("int");
    params.set_output("float", float(i));
    std::cout << "The invisible node is being executed!" << std::endl;
}

void node_exec_Texture_to_NumpyArray(ExeParams exe_params)
{
    auto handle = exe_params.get_input<TextureHandle>("Texture");

    std::vector shape{ handle->getDesc().width, handle->getDesc().height };
    std::vector<unsigned> stride{ sizeof(float), sizeof(float) };

    auto m_CommandList = resource_allocator.create(CommandListDesc{});

    auto staging = resource_allocator.create(
        StagingTextureDesc{ handle->getDesc() }, nvrhi::CpuAccessMode::Read);

    auto nvrhi_device = resource_allocator.device;
    auto width = handle->getDesc().width;
    auto height = handle->getDesc().height;
    auto s = boost::python::make_tuple(width, height, 4u);
    auto arr = bpn::empty(s, bpn::dtype::get_builtin<float>());

    m_CommandList->open();

    m_CommandList->copyTexture(staging, {}, handle, {});
    m_CommandList->close();

    nvrhi_device->executeCommandList(m_CommandList.Get());

    size_t pitch;
    auto mapped = nvrhi_device->mapStagingTexture(
        staging, {}, nvrhi::CpuAccessMode::Read, &pitch);

    auto hd_format = HdFormat_from_nvrhi_format(handle->getDesc().format);
    for (int i = 0; i < handle->getDesc().height; ++i) {
        auto copy_to_loc =
            arr.get_data() + i * width * HdDataSizeOfFormat(hd_format);
        memcpy(
            copy_to_loc,
            (uint8_t*)mapped + i * pitch,
            width * pxr::HdDataSizeOfFormat(hd_format));
    }

    nvrhi_device->unmapStagingTexture(staging);

    resource_allocator.destroy(m_CommandList);
    resource_allocator.destroy(staging);
    exe_params.set_output("NumpyArray", arr);
}

void node_declare_Texture_to_NumpyArray(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Texture>("Texture");
    b.add_output<decl::NumpyArray>("NumpyArray");
}

void node_declare_NumpyArray_to_Texture(NodeDeclarationBuilder& b)
{
    b.add_input<decl::NumpyArray>("NumpyArray");
    b.add_output<decl::Texture>("Texture");
}

void node_exec_NumpyArray_to_Texture(ExeParams exe_params)
{
    auto np_array = exe_params.get_input<bpn::ndarray>("NumpyArray");

    if (np_array.get_dtype() != bpn::dtype::get_builtin<float>()) {
        throw std::runtime_error("Numpy array must have dtype float.");
    }

    auto shape = np_array.get_shape();
    if (np_array.get_nd() != 3 || shape[2] != 4) {
        throw std::runtime_error(
            "Numpy array must have shape (height, width, 4).");
    }

    auto width = shape[0];
    auto height = shape[1];

    auto tex_desc =
        nvrhi::TextureDesc().setWidth(width).setHeight(height).setFormat(
            nvrhi::Format::RGBA32_FLOAT);

    tex_desc.initialState = nvrhi::ResourceStates::CopyDest;
    tex_desc.keepInitialState = true;
    tex_desc.isUAV = true;

    auto texture = resource_allocator.create(tex_desc);
    auto staging = resource_allocator.create(
        StagingTextureDesc{ tex_desc }, nvrhi::CpuAccessMode::Write);

    auto m_CommandList = resource_allocator.create(CommandListDesc{});
    auto nvrhi_device = resource_allocator.device;

    size_t pitch;
    auto mapped = nvrhi_device->mapStagingTexture(
        staging, {}, nvrhi::CpuAccessMode::Write, &pitch);

    auto hd_format = HdFormat_from_nvrhi_format(tex_desc.format);

    for (int i = 0; i < height; ++i) {
        auto copy_from_loc =
            np_array.get_data() + i * width * HdDataSizeOfFormat(hd_format);
        memcpy(
            (uint8_t*)mapped + i * pitch,
            copy_from_loc,
            width * pxr::HdDataSizeOfFormat(hd_format));
    }

    nvrhi_device->unmapStagingTexture(staging);

    m_CommandList->open();
    m_CommandList->copyTexture(texture, {}, staging, {});
    m_CommandList->close();

    nvrhi_device->executeCommandList(m_CommandList.Get());

    resource_allocator.destroy(m_CommandList);
    resource_allocator.destroy(staging);

    exe_params.set_output("Texture", texture);
}

void node_declare_NumpyArray_to_Buffer(NodeDeclarationBuilder& b)
{
    b.add_input<decl::NumpyArray>("NumpyArray");
    b.add_output<nvrhi::BufferHandle>("Buffer");
}

void node_exec_NumpyArray_to_Buffer(ExeParams params)
{
    auto np_array = params.get_input<bpn::ndarray>("NumpyArray");

    if (np_array.get_dtype() != bpn::dtype::get_builtin<float>()) {
        throw std::runtime_error("Numpy array must have dtype float.");
    }

    auto shape = np_array.get_shape();

    auto byte_size = 1;

    for (int i = 0; i < np_array.get_nd(); ++i) {
        byte_size *= shape[i];
    }

    for (int i = 0; i < np_array.get_nd() - 1; ++i) {
        assert(np_array.strides(i) > np_array.strides(i + 1));
        // Make sure there are no surprises!
    }

    byte_size *= np_array.strides(-1);

    auto buffer_desc =
        nvrhi::BufferDesc()
            .setByteSize(byte_size)
            .setFormat(nvrhi::Format::R32_FLOAT)
            .setCpuAccess(nvrhi::CpuAccessMode::Write);  // Just first use a
                                                         // long float buffer

    buffer_desc.initialState = nvrhi::ResourceStates::CopyDest;
    buffer_desc.keepInitialState = true;

    auto buffer = resource_allocator.create(buffer_desc);
    auto nvrhi_device = resource_allocator.device;
    auto mapped = nvrhi_device->mapBuffer(buffer, nvrhi::CpuAccessMode::Write);
    memcpy(mapped, np_array.get_data(), byte_size);
    nvrhi_device->unmapBuffer(buffer);

    params.set_output("Buffer", buffer);
}

#if USTC_CG_WITH_TORCH

void node_declare_NumpyArray_to_TorchTensor(NodeDeclarationBuilder& b)
{
    b.add_input<decl::NumpyArray>("arr");
    b.add_output<decl::TorchTensor>("tensor");
}

inline torch::Dtype convert_dtype(const boost::python::numpy::dtype& np_dtype)
{
    if (np_dtype == boost::python::numpy::dtype::get_builtin<float>()) {
        return torch::kFloat32;
    }
    else if (np_dtype == boost::python::numpy::dtype::get_builtin<double>()) {
        return torch::kFloat64;
    }
    else if (np_dtype == boost::python::numpy::dtype::get_builtin<int>()) {
        return torch::kInt32;
    }
    else if (np_dtype == boost::python::numpy::dtype::get_builtin<long>()) {
        return torch::kInt64;
    }
    else {
        throw std::invalid_argument("Unsupported numpy dtype");
    }
}

void node_exec_NumpyArray_to_TorchTensor(ExeParams params)
{
    namespace np = boost::python::numpy;

    // Get the input numpy array
    np::ndarray arr = params.get_input<np::ndarray>("arr");

    // Get the shape of the numpy array
    Py_intptr_t const* shape = arr.get_shape();
    int ndim = arr.get_nd();

    // Convert the shape to a vector
    std::vector<int64_t> tensor_shape(shape, shape + ndim);

    // Get the data type of the numpy array
    torch::Dtype dtype = convert_dtype(arr.get_dtype());

    at::TensorOptions options = at::TensorOptions().dtype(dtype);

    // Create a torch tensor from the numpy array data
    torch::Tensor tensor = torch::from_blob(
        arr.get_data(), torch::IntArrayRef(tensor_shape), options);
    at::Tensor device_tensor = tensor.to(at::Device(at::DeviceType::CUDA, 0));

    if (!device_tensor.is_cuda()) {
        std::cout << "Fuck, already fucked in C++" << std::endl;
    }

    // Set the tensor as output
    params.set_output("tensor", device_tensor);
}

void node_declare_TorchTensor_to_Texture(NodeDeclarationBuilder& b)
{
    b.add_input<decl::TorchTensor>("tensor");
    b.add_output<decl::Texture>("texture");
}

void node_exec_TorchTensor_to_Texture(ExeParams exe_params)
{
    auto tensor = exe_params.get_input<torch::Tensor>("tensor");

    if (tensor.dtype() != torch::kFloat) {
        throw std::runtime_error("Numpy array must have dtype float.");
    }

    auto shape = tensor.sizes();
    if (tensor.sizes().size() != 3 || shape[2] != 4) {
        throw std::runtime_error(
            "Numpy array must have shape (height, width, 4).");
    }

    auto width = shape[1];
    auto height = shape[0];

    auto tex_desc =
        nvrhi::TextureDesc()
            .setWidth(width)
            .setHeight(height)
            .setFormat(nvrhi::Format::RGBA32_FLOAT)
            .setMipLevels(1)
            .setSharedResourceFlags(nvrhi::SharedResourceFlags::Shared);

    tex_desc.initialState = nvrhi::ResourceStates::CopyDest;
    tex_desc.keepInitialState = true;
    tex_desc.isUAV = true;

    auto texture = resource_allocator.create(tex_desc);

    auto device = resource_allocator.device;
    cudaExternalMemory_t externalMemory;
    cudaMipmappedArray_t mipmapArray;
    CUsurfObject cuda_ptr =
        mapTextureToSurface(texture, 0, mipmapArray, externalMemory, device);

    BlitLinearBufferToSurface(
        static_cast<float4*>(tensor.data_ptr()), cuda_ptr, width, height);

    cudaFreeMipmappedArray(mipmapArray);
    cudaDestroyExternalMemory(externalMemory);

    exe_params.set_output("texture", texture);
}

void node_declare_Float1Buffer_to_TorchTensor(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Float1Buffer>("buffer");
    b.add_output<decl::TorchTensor>("tensor");
}

void node_exec_Float1Buffer_to_TorchTensor(ExeParams params)
{
    pxr::VtArray<float> buffer =
        params.get_input<pxr::VtArray<float>>("buffer");
    long long width = buffer.size();
    auto tensor = torch::empty({ width }, torch::kFloat);
    memcpy(tensor.data_ptr(), buffer.data(), width * sizeof(float));

    auto device_tensor = tensor.to(at::Device(at::DeviceType::CUDA, 0));

    params.set_output("tensor", device_tensor);
}
#endif

void node_declare_Float1Buffer_to_NumpyArray(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Float1Buffer>("buffer");
    b.add_output<decl::NumpyArray>("arr");
}

void node_exec_Float1Buffer_to_NumpyArray(ExeParams params)
{
    pxr::VtArray<float> buffer =
        params.get_input<pxr::VtArray<float>>("buffer");
    auto width = buffer.size();
    auto s = boost::python::make_tuple(width);
    auto arr = bpn::empty(s, bpn::dtype::get_builtin<float>());
    memcpy(arr.get_data(), buffer.data(), width * sizeof(float));
    params.set_output("arr", arr);
}

void node_declare_Float_to_NumpyArray(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Float>("float");
    b.add_output<decl::NumpyArray>("arr");
}

void node_exec_Float_to_NumpyArray(ExeParams params)
{
    auto f = params.get_input<float>("float");
    auto s = boost::python::make_tuple(1);
    auto arr = bpn::empty(s, bpn::dtype::get_builtin<float>());
    *reinterpret_cast<float*>(arr.get_data()) = f;
    params.set_output("arr", arr);
}

static void node_register()
{
#define CONVERSION(FROM, TO)                                                 \
    static NodeTypeInfo ntype_##FROM##_to_##TO;                              \
    strcpy(ntype_##FROM##_to_##TO.ui_name, "invisible");                     \
    strcpy(ntype_##FROM##_to_##TO.id_name, "conv_" #FROM "_to_" #TO);        \
    func_node_type_base(&ntype_##FROM##_to_##TO);                            \
    ntype_##FROM##_to_##TO.node_execute = node_exec_##FROM##_to_##TO;        \
    ntype_##FROM##_to_##TO.declare = node_declare_##FROM##_to_##TO;          \
    ntype_##FROM##_to_##TO.node_type_of_grpah = NodeTypeOfGrpah::Conversion; \
    ntype_##FROM##_to_##TO.INVISIBLE = true;                                 \
    ntype_##FROM##_to_##TO.conversion_from = SocketType::FROM;               \
    ntype_##FROM##_to_##TO.conversion_to = SocketType::TO;                   \
    nodeRegisterType(&ntype_##FROM##_to_##TO);

    CONVERSION(Int, Float)
    CONVERSION(Texture, NumpyArray)
    CONVERSION(NumpyArray, Texture)
    CONVERSION(NumpyArray, Buffer)

#if USTC_CG_WITH_TORCH
    CONVERSION(NumpyArray, TorchTensor)
    CONVERSION(TorchTensor, Texture)
    CONVERSION(Float1Buffer, TorchTensor)
#endif

    CONVERSION(Float1Buffer, NumpyArray)
    CONVERSION(Float, NumpyArray)
}


}  // namespace USTC_CG::node_conversion
