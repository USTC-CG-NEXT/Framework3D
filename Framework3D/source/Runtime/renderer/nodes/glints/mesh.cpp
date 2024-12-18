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
    const std::vector<float>& world_to_clip)
{
    std::cout << "vertices " << vertices << std::endl;
    std::cout << "indices " << indices << std::endl;
    std::cout << "vertices_count " << vertices_count << std::endl;
    std::cout << "vertex_buffer_stride " << vertex_buffer_stride << std::endl;
    std::cout << "index_count " << index_count << std::endl;
    std::cout << "resolution " << resolution.x << " " << resolution.y
              << std::endl;

    assert(vertices);
    assert(indices);
    auto vertex_buffer_desc = cuda::CUDALinearBufferDesc{
        static_cast<unsigned>(
            vertices_count * vertex_buffer_stride / sizeof(float)),
        sizeof(float)
    };
    vertex_buffer =
        cuda::borrow_cuda_linear_buffer(vertex_buffer_desc, vertices);

    auto host_vb = vertex_buffer->get_host_vector<float>();
    for (int i = 0; i < host_vb.size(); ++i) {
        std::cout << host_vb[i] << " ";
    }
    std::cout << std::endl;
    auto index_buffer_desc =
        cuda::CUDALinearBufferDesc{ index_count, sizeof(unsigned) };
    index_buffer = cuda::borrow_cuda_linear_buffer(index_buffer_desc, indices);
    auto host_ib = index_buffer->get_host_vector<unsigned>();
    for (int i = 0; i < host_ib.size(); ++i) {
        std::cout << host_ib[i] << " ";
    }
    std::cout << std::endl;

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
    corners_buffer = cuda::create_cuda_linear_buffer<Corners>(ray_count);

    ensure_pipeline();

    float4x4 worldToClip;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            worldToClip.m[i][j] = world_to_clip[i * 4 + j];
        }
    }

    auto mesh_params = cuda::create_cuda_linear_buffer<MeshTracingParams>(
        MeshTracingParams{ handle->getOptiXTraversable(),
                           (float*)vertex_buffer->get_device_ptr(),
                           (unsigned*)index_buffer->get_device_ptr(),
                           append_buffer.get_device_queue_ptr(),
                           (Corners*)corners_buffer->get_device_ptr(),
                           (int2*)target_buffer->get_device_ptr(),
                           worldToClip });

    cuda::optix_trace_ray<MeshTracingParams>(
        handle,
        pipeline,
        mesh_params->get_device_ptr(),
        resolution.x,
        resolution.y,
        1);

    return std::make_tuple(
        reinterpret_cast<float*>(append_buffer.get_underlying_buffer_ptr()),
        reinterpret_cast<float*>(append_buffer.get_underlying_buffer_ptr()),
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