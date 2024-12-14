#pragma once
#include <nvrhi/nvrhi.h>

#include "../source/camera.h"
#include "../source/geometries/mesh.h"
#include "../source/light.h"
#include "../source/material/material.h"
#include "RHI/ResourceManager/resource_allocator.hpp"
#include "RHI/internal/resources.hpp"
#include "hd_USTC_CG/render_global_payload.hpp"
#include "nodes/core/node_exec.hpp"
#include "nvrhi/nvrhi.h"
#include "renderer/program_vars.hpp"
#include "shaders/shaders/utils/view_cb.h"
#include "utils/cam_to_view_contants.h"
#include "utils/resource_cleaner.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
inline Hd_USTC_CG_Camera* get_free_camera(
    ExeParams& params,
    const std::string& camera_name = "Camera")
{
    auto& cameras =
        params.get_global_payload<RenderGlobalPayload&>().get_cameras();

    Hd_USTC_CG_Camera* free_camera = nullptr;
    for (auto camera : cameras) {
        if (camera->GetId() != SdfPath::EmptyPath()) {
            free_camera = camera;
            break;
        }
    }
    return free_camera;
}

#define global_payload params.get_global_payload<RenderGlobalPayload&>()
#define instance_collection global_payload.InstanceCollection
inline ResourceAllocator& get_resource_allocator(ExeParams& params)
{
    return params.get_global_payload<RenderGlobalPayload&>().resource_allocator;
}

inline ShaderFactory& get_shader_factory(ExeParams& params)
{
    return params.get_global_payload<RenderGlobalPayload&>().shader_factory;
}

#define resource_allocator get_resource_allocator(params)
#define shader_factory     get_shader_factory(params)

inline BufferHandle get_free_camera_planarview_cb(
    ExeParams& params,
    const std::string& camera_name = "Camera")
{
    auto free_camera = get_free_camera(params, camera_name);
    auto constant_buffer = resource_allocator.create(
        BufferDesc{ .byteSize = sizeof(PlanarViewConstants),
                    .debugName = "constantBuffer",
                    .isConstantBuffer = true,
                    .initialState = nvrhi::ResourceStates::ConstantBuffer,
                    .cpuAccess = nvrhi::CpuAccessMode::Write });

    PlanarViewConstants view_constant = camera_to_view_constants(free_camera);
    auto mapped_constant_buffer = resource_allocator.device->mapBuffer(
        constant_buffer, nvrhi::CpuAccessMode::Write);
    memcpy(mapped_constant_buffer, &view_constant, sizeof(PlanarViewConstants));
    resource_allocator.device->unmapBuffer(constant_buffer);
    return constant_buffer;
}

inline BufferHandle get_model_buffer(
    ExeParams& params,
    const pxr::GfMatrix4f& matrix)
{
    auto desc =
        BufferDesc{ .byteSize = sizeof(pxr::GfMatrix4f),
                    .debugName = "modelBuffer",
                    .isConstantBuffer = true,
                    .initialState = nvrhi::ResourceStates::ConstantBuffer,
                    .cpuAccess = nvrhi::CpuAccessMode::Write };
    desc.structStride = sizeof(pxr::GfMatrix4f);
    auto model_buffer = resource_allocator.create(desc);

    auto mapped_model_buffer = resource_allocator.device->mapBuffer(
        model_buffer, nvrhi::CpuAccessMode::Write);
    memcpy(mapped_model_buffer, &matrix, sizeof(pxr::GfMatrix4f));
    resource_allocator.device->unmapBuffer(model_buffer);
    return model_buffer;
}

template<typename T>
inline BufferHandle create_buffer(
    ExeParams& params,
    size_t count,
    bool is_constant_buffer = false,
    bool is_uav_buffer = false,
    bool isVertexBuffer = false)
{
    nvrhi::BufferDesc buffer_desc = nvrhi::BufferDesc();
    buffer_desc.byteSize = count * sizeof(T);
    buffer_desc.isVertexBuffer = isVertexBuffer;
    buffer_desc.initialState = nvrhi::ResourceStates::ShaderResource;
    buffer_desc.debugName = type_name<T>().data();
    buffer_desc.structStride = sizeof(T);
    buffer_desc.keepInitialState = true;

    if (is_constant_buffer) {
        buffer_desc.isConstantBuffer = true;
        buffer_desc.initialState = nvrhi::ResourceStates::ConstantBuffer;
        buffer_desc.cpuAccess = nvrhi::CpuAccessMode::Write;
    }

    if (is_uav_buffer) {
        buffer_desc.canHaveUAVs = true;
        buffer_desc.initialState = nvrhi::ResourceStates::UnorderedAccess;
    }

    auto buffer = resource_allocator.create(buffer_desc);

    return buffer;
}

template<typename T>
inline BufferHandle create_buffer(
    ExeParams& params,
    size_t count,
    const T& init_value,
    bool is_constant_buffer = false,
    bool is_uav_buffer = false)
{
    auto buffer =
        create_buffer<T>(params, count, is_constant_buffer, is_uav_buffer);

    // fill the buffer with default values
    std::vector<T> cpu_data(count, init_value);
    auto ptr = resource_allocator.device->mapBuffer(
        buffer, nvrhi::CpuAccessMode::Write);

    memcpy(ptr, cpu_data.data(), cpu_data.size() * sizeof(T));

    resource_allocator.device->unmapBuffer(buffer);

    return buffer;
}

template<typename T>
inline BufferHandle create_constant_buffer(ExeParams& params, const T& value)
{
    return create_buffer<T>(params, 1, value, true);
}

template<typename T>
inline BufferHandle create_uav_buffer(  // unordered access view
    ExeParams& params,
    size_t count,
    const T& init_value)
{
    return create_buffer<T>(params, count, init_value, false, true);
}

template<typename T>
inline std::tuple<nvrhi::BufferHandle, nvrhi::BufferHandle>
create_counter_buffer(ExeParams& params, size_t max_size)
{
    nvrhi::BufferDesc storage_buffer = nvrhi::BufferDesc();
    storage_buffer.byteSize = max_size * sizeof(T);
    storage_buffer.initialState = nvrhi::ResourceStates::UnorderedAccess;
    storage_buffer.debugName = type_name<T>().data();
    storage_buffer.cpuAccess = nvrhi::CpuAccessMode::Write;
    storage_buffer.canHaveUAVs = true;
    storage_buffer.structStride = sizeof(T);
    auto buffer = resource_allocator.create(storage_buffer);

    nvrhi::BufferDesc counter_buffer = nvrhi::BufferDesc();
    counter_buffer.byteSize = sizeof(uint32_t);
    counter_buffer.isVertexBuffer = true;
    counter_buffer.initialState = nvrhi::ResourceStates::UnorderedAccess;
    counter_buffer.debugName = "counterBuffer";
    counter_buffer.cpuAccess = nvrhi::CpuAccessMode::Write;
    counter_buffer.structStride = sizeof(uint32_t);
    counter_buffer.canHaveUAVs = true;
    auto counter = resource_allocator.create(counter_buffer);

    // fill the counter with 0
    uint32_t zero = 0;
    auto ptr = resource_allocator.device->mapBuffer(
        counter, nvrhi::CpuAccessMode::Write);
    memcpy(ptr, &zero, sizeof(uint32_t));
    resource_allocator.device->unmapBuffer(counter);

    return { buffer, counter };
}

inline unsigned counter_read_out(ExeParams& params, nvrhi::IBuffer* counter)
{
    uint32_t count = 0;
    resource_allocator.device->waitForIdle();
    auto ptr = resource_allocator.device->mapBuffer(
        counter, nvrhi::CpuAccessMode::Read);
    memcpy(&count, ptr, sizeof(uint32_t));
    resource_allocator.device->unmapBuffer(counter);
    return count;
}

inline TextureHandle create_default_render_target(
    ExeParams& params,
    nvrhi::Format format = nvrhi::Format::RGBA16_FLOAT)
{
    auto camera = get_free_camera(params);
    auto size = camera->dataWindow.GetSize();
    // Output texture
    nvrhi::TextureDesc desc =
        nvrhi::TextureDesc{}
            .setWidth(size[0])
            .setHeight(size[1])
            .setFormat(format)
            .setInitialState(nvrhi::ResourceStates::RenderTarget)
            .setKeepInitialState(true)
            .setIsRenderTarget(true);
    auto output_texture = resource_allocator.create(desc);
    return output_texture;
}

inline void initialize_texture(
    ExeParams& params,
    nvrhi::ITexture* texture,
    const nvrhi::Color& color)
{
    auto command_list = resource_allocator.create(CommandListDesc{});
    command_list->open();
    command_list->clearTextureFloat(texture, {}, color);
    command_list->close();
    resource_allocator.device->executeCommandList(command_list);
    resource_allocator.destroy(command_list);
}

inline TextureHandle create_default_depth_stencil(ExeParams& params)
{
    auto camera = get_free_camera(params);
    auto size = camera->dataWindow.GetSize();
    // Depth texture
    nvrhi::TextureDesc depth_desc =
        nvrhi::TextureDesc{}
            .setWidth(size[0])
            .setHeight(size[1])
            .setFormat(nvrhi::Format::D32)
            .setIsRenderTarget(true)
            .setInitialState(nvrhi::ResourceStates::DepthWrite)
            .setKeepInitialState(true);
    auto depth_stencil_texture = resource_allocator.create(depth_desc);
    return depth_stencil_texture;
}

inline auto get_size(ExeParams& params)
{
    auto camera = get_free_camera(params);
    auto size = camera->dataWindow.GetSize();
    return size;
}

#define CHECK_PROGRAM_ERROR(program)              \
    if (!program->get_error_string().empty()) {   \
        log::warning(                             \
            "Failed to create shader %s: %s",     \
            #program,                             \
            program->get_error_string().c_str()); \
        return false;                             \
    }


#ifdef _DEBUG
#define PROFILE_SCOPE(node_name) auto scope = log::profile_scope(#node_name)
#else
#define PROFILE_SCOPE(node_name)
#endif

USTC_CG_NAMESPACE_CLOSE_SCOPE