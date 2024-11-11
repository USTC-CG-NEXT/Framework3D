#pragma once

#include "../source/camera.h"
#include "../source/geometries/mesh.h"
#include "../source/light.h"
#include "../source/material.h"
#include "RHI/ResourceManager/resource_allocator.hpp"
#include "hd_USTC_CG/render_global_payload.hpp"
#include "nodes/core/node_exec.hpp"
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

inline BufferHandle get_free_camera_cb(
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

inline TextureHandle create_default_render_target(
    ExeParams& params,
    nvrhi::Format format = nvrhi::Format::RGBA8_UNORM)
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

USTC_CG_NAMESPACE_CLOSE_SCOPE