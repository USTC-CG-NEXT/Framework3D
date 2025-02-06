#if USTC_CG_WITH_CUDA

#include "mesh.hpp"

#include <complex.h>

#include "../shaders/shaders/glints/mesh_params.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

MeshIntersectionContext::~MeshIntersectionContext()
{
    raygen_group = nullptr;
    hg_module = nullptr;
    hg = nullptr;
    miss_group = nullptr;
    pipeline = nullptr;
    vertex_buffer = nullptr;
    index_buffer = nullptr;
}

std::tuple<float*, float*, unsigned*, unsigned>
MeshIntersectionContext::intersect_mesh_with_rays(
    float* vertices,
    unsigned vertices_count,
    unsigned vertex_buffer_stride,
    unsigned* indices,
    unsigned index_count,
    int2 resolution,
    const std::vector<float>& world_to_view,
    const std::vector<float>& view_to_clip)
{
    assert(vertices);
    assert(indices);
    auto vertex_buffer_desc = cuda::CUDALinearBufferDesc{
        static_cast<unsigned>(
            vertices_count * vertex_buffer_stride / sizeof(float)),
        sizeof(float)
    };
    vertex_buffer =
        cuda::borrow_cuda_linear_buffer(vertex_buffer_desc, vertices);

    auto index_buffer_desc =
        cuda::CUDALinearBufferDesc{ index_count, sizeof(unsigned) };
    index_buffer = cuda::borrow_cuda_linear_buffer(index_buffer_desc, indices);

    assert(index_count % 3 == 0);
    handle = cuda::create_mesh_optix_traversable(
        { vertex_buffer->get_device_ptr() },
        vertices_count,
        vertex_buffer_stride,
        { index_buffer->get_device_ptr() },
        index_count / 3);

    auto ray_count = resolution.x * resolution.y;

    append_buffer = cuda::AppendStructuredBuffer<Patch>(ray_count);
    target_buffer = cuda::create_cuda_linear_buffer<int2>(ray_count);
    worldToUV = cuda::create_cuda_linear_buffer<float4x4>(ray_count);

    ensure_pipeline();

    float4x4 worldToView;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            worldToView.m[i][j] = world_to_view[i * 4 + j];
        }
    }

    float4x4 viewToClip;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            viewToClip.m[i][j] = view_to_clip[i * 4 + j];
        }
    }

    auto mesh_params = cuda::create_cuda_linear_buffer<MeshTracingParams>(
        MeshTracingParams{ handle->getOptiXTraversable(),
                           (float*)vertex_buffer->get_device_ptr(),
                           (unsigned*)index_buffer->get_device_ptr(),
                           append_buffer.get_device_queue_ptr(),
                           (float4x4*)worldToUV->get_device_ptr(),
                           (int2*)target_buffer->get_device_ptr(),
                           worldToView,
                           viewToClip });

    cuda::optix_trace_ray<MeshTracingParams>(
        handle,
        pipeline,
        mesh_params->get_device_ptr(),
        resolution.x,
        resolution.y,
        1);

    return std::make_tuple(
        reinterpret_cast<float*>(append_buffer.get_underlying_buffer_ptr()),
        reinterpret_cast<float*>(worldToUV->get_device_ptr()),
        reinterpret_cast<unsigned*>(target_buffer->get_device_ptr()),
        append_buffer.get_size());
}

void MeshIntersectionContext::create_raygen(const std::string& string)
{
    if (!raygen_group) {
        raygen_group =
            cuda::create_optix_raygen(string, RGS_STR(mesh), "mesh_params");
    }
}

void MeshIntersectionContext::create_hitgroup_module(const std::string& string)
{
    if (!hg_module) {
        hg_module = cuda::create_optix_module(string, "mesh_params");
    }
}

void MeshIntersectionContext::create_hitgroup()
{
    if (!hg) {
        cuda::OptiXProgramGroupDesc hg_desc;
        hg_desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_HITGROUP)
            .set_entry_name(nullptr, AHS_STR(mesh), CHS_STR(mesh));
        hg = create_optix_program_group(
            hg_desc, { nullptr, hg_module, hg_module });
    }
}

void MeshIntersectionContext::create_miss_group(const std::string& string)
{
    if (!miss_group) {
        miss_group =
            cuda::create_optix_miss(string, MISS_STR(mesh), "mesh_params");
    }
}

void MeshIntersectionContext::create_pipeline()
{
    if (!pipeline) {
        pipeline = cuda::create_optix_pipeline(
            { raygen_group, hg, miss_group }, "mesh_params");
    }
}

void MeshIntersectionContext::ensure_pipeline()
{
    std::string filename =
        RENDERER_SHADER_DIR + std::string("shaders/glints/mesh.cu");

    create_raygen(filename);
    create_hitgroup_module(filename);
    create_hitgroup();
    create_miss_group(filename);
    create_pipeline();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE

#endif // USTC_CG_WITH_CUDA