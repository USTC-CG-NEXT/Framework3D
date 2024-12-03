#pragma once
#include <RHI/api.h>
#include <nvrhi/nvrhi.h>
#include <thrust/host_vector.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "optix/optix.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

namespace cuda {
RHI_API int cuda_init();
RHI_API int optix_init();
RHI_API int cuda_shutdown();

using OptixTraversableHandle = unsigned long long;
class OptiXTraversableDesc;

class IOptiXTraversable : public nvrhi::IResource {
   public:
    [[nodiscard]] virtual const OptiXTraversableDesc& getDesc() const = 0;

   protected:
    virtual OptixTraversableHandle getOptiXTraversable() const = 0;
};

using OptiXTraversableHandle = nvrhi::RefCountPtr<IOptiXTraversable>;

class OptiXTraversableDesc {
   public:
    OptixBuildInput buildInput = {};
    OptixAccelBuildOptions buildOptions = {};

    std::vector<OptiXTraversableHandle> handles;
};

RHI_API OptiXTraversableHandle
create_optix_traversable(const OptiXTraversableDesc& d);

RHI_API OptiXTraversableHandle create_optix_traversable(
    std::vector<CUdeviceptr> vertexBuffer,
    unsigned int numVertices,
    std::vector<CUdeviceptr> widthBuffer,
    CUdeviceptr indexBuffer,
    unsigned int numPrimitives,
    bool rebuilding = false);

struct CUDALinearBufferDesc {
    int size;
    int element_size;

    CUDALinearBufferDesc(int size = 0, int element_size = 0)
        : size(size),
          element_size(element_size)
    {
    }

    friend class CUDALinearBuffer;
};

class ICUDALinearBuffer : public nvrhi::IResource {
   public:
    virtual ~ICUDALinearBuffer() = default;
    [[nodiscard]] virtual const CUDALinearBufferDesc& getDesc() const = 0;
    virtual CUdeviceptr get_device_ptr() = 0;

    template<typename T>
    std::vector<T> get_host_vector()
    {
        auto host_data = get_host_data();
        auto data_ptr = host_data.data();
        auto size = getDesc().size;

        auto ret = std::vector<T>(
            reinterpret_cast<T*>(data_ptr),
            reinterpret_cast<T*>(data_ptr) + size);
        return ret;
    }

    template<typename T>
    void assign_host_vector(const std::vector<T>& data)
    {
        auto host_data = thrust::host_vector<uint8_t>(
            reinterpret_cast<const uint8_t*>(data.data()),
            reinterpret_cast<const uint8_t*>(data.data() + data.size()));
        assign_host_data(host_data);
    }

   protected:
    virtual thrust::host_vector<uint8_t> get_host_data() = 0;
    virtual void assign_host_data(const thrust::host_vector<uint8_t>& data) = 0;
};
using CUDALinearBufferHandle = nvrhi::RefCountPtr<ICUDALinearBuffer>;
RHI_API CUDALinearBufferHandle
create_cuda_linear_buffer(const CUDALinearBufferDesc& d);

template<typename T>
CUDALinearBufferHandle create_cuda_linear_buffer(const std::vector<T>& d)
{
    CUDALinearBufferDesc desc(d.size(), sizeof(T));
    auto ret = create_cuda_linear_buffer(desc);
    ret->assign_host_vector(d);
    return ret;
}

class OptiXProgramGroupDesc {
   public:
    OptixProgramGroupOptions program_group_options = {};
    OptixProgramGroupDesc prog_group_desc;
};

class OptiXPipelineDesc {
   public:
    OptixPipelineCompileOptions pipeline_compile_options;
    OptixPipelineLinkOptions pipeline_link_options;
};

class OptiXModuleDesc {
   public:
    OptixModuleCompileOptions module_compile_options;
    OptixPipelineCompileOptions pipeline_compile_options;
    OptixBuiltinISOptions builtinISOptions;

    std::string ptx;
};

class IOptiXProgramGroup : public nvrhi::IResource {
   public:
    [[nodiscard]] virtual const OptiXProgramGroupDesc& getDesc() const = 0;

   protected:
    virtual OptixProgramGroup getProgramGroup() const = 0;
    friend class OptiXPipeline;
};

class IOptiXModule : public nvrhi::IResource {
   public:
    [[nodiscard]] virtual const OptiXModuleDesc& getDesc() const = 0;

   protected:
    virtual OptixModule getModule() const = 0;
    friend class OptiXProgramGroup;
};

class IOptiXPipeline : public nvrhi::IResource {
   public:
    [[nodiscard]] virtual const OptiXPipelineDesc& getDesc() const = 0;

   protected:
    virtual OptixPipeline getPipeline() const = 0;
};
using OptiXModuleHandle = nvrhi::RefCountPtr<IOptiXModule>;
using OptiXPipelineHandle = nvrhi::RefCountPtr<IOptiXPipeline>;
using OptiXProgramGroupHandle = nvrhi::RefCountPtr<IOptiXProgramGroup>;

OptiXModuleHandle create_optix_module(const OptiXModuleDesc& d);

OptiXProgramGroupHandle create_optix_program_group(
    const OptiXProgramGroupDesc& d,
    OptiXModuleHandle module);

OptiXProgramGroupHandle create_optix_program_group(
    const OptiXProgramGroupDesc& d,
    std::tuple<OptiXModuleHandle, OptiXModuleHandle, OptiXModuleHandle>
        modules);

OptiXProgramGroupHandle create_optix_raygen(const std::string& file_path, const std::string& entry_name);

OptiXPipelineHandle create_optix_pipeline(
    const OptiXPipelineDesc& d,
    std::vector<OptiXProgramGroupHandle> program_groups = {});



}  // namespace cuda

USTC_CG_NAMESPACE_CLOSE_SCOPE