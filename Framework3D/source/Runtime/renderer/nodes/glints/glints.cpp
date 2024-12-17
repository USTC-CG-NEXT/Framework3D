#include "glints.hpp"

#include <complex.h>

#include "../shaders/shaders/glints/params.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
void ScratchIntersectionContext::create_raygen(std::string filename)
{
    if (!raygen_group) {
        raygen_group = cuda::create_optix_raygen(filename, RGS_STR(line));
    }
}

void ScratchIntersectionContext::create_cylinder_intersection_shader()
{
    if (!cylinder_module) {
        cylinder_module =
            cuda::get_builtin_module(OPTIX_PRIMITIVE_TYPE_ROUND_LINEAR);
    }
}

void ScratchIntersectionContext::create_hitgroup_module(std::string filename)
{
    if (!hg_module) {
        hg_module = cuda::create_optix_module(filename);
    }
}

void ScratchIntersectionContext::create_hitgroup()
{
    if (!hg) {
        cuda::OptiXProgramGroupDesc hg_desc;
        hg_desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_HITGROUP)
            .set_entry_name(nullptr, AHS_STR(line), CHS_STR(line));
        hg = create_optix_program_group(
            hg_desc, { cylinder_module, hg_module, hg_module });
    }
}

void ScratchIntersectionContext::create_miss_group(std::string filename)
{
    if (!miss_group) {
        miss_group = cuda::create_optix_miss(filename, MISS_STR(line));
    }
}

void ScratchIntersectionContext::create_pipeline()
{
    if (!pipeline) {
        pipeline =
            cuda::create_optix_pipeline({ raygen_group, hg, miss_group });
    }
}

void ScratchIntersectionContext::create_width_buffer(
    unsigned vertex_count,
    float width)
{
    if (!widths || width != _width || vertex_count != _vertex_count) {
        widths =
            cuda::create_cuda_linear_buffer(std::vector(vertex_count, width));
        _width = width;
    }
}

void ScratchIntersectionContext::create_indices(unsigned vertex_count)
{
    if (!indices || vertex_count != _vertex_count) {
        std::vector<unsigned> h_indices;
        h_indices.reserve(vertex_count);
        for (int i = 0; i < vertex_count / 2; ++i) {
            h_indices.push_back(i * 2);
        }
        indices = cuda::create_cuda_linear_buffer(h_indices);
    }
}

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
    float* indices,
    unsigned index_count,
    int2 resolution,
    const std::vector<float>& world_to_clip)
{
    auto vertex_buffer_desc =
        cuda::CUDALinearBufferDesc{ vertices_count * vertex_buffer_stride,
                                    sizeof(float) };
    vertex_buffer =
        cuda::borrow_cuda_linear_buffer(vertex_buffer_desc, vertices);
    auto index_buffer_desc =
        cuda::CUDALinearBufferDesc{ index_count, sizeof(unsigned) };
    index_buffer = cuda::borrow_cuda_linear_buffer(index_buffer_desc, indices);

    handle = cuda::create_mesh_optix_traversable(
        { vertex_buffer->get_device_ptr() },
        vertices_count,
        vertex_buffer_stride,
        { index_buffer->get_device_ptr() },
        index_count);

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
        raygen_group = cuda::create_optix_raygen(string, RGS_STR(mesh));
    }
}

void MeshIntersectionContext::create_hitgroup_module(const std::string& string)
{
    if (!hg_module) {
        hg_module = cuda::create_optix_module(string);
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
        miss_group = cuda::create_optix_miss(string, MISS_STR(mesh));
    }
}

void MeshIntersectionContext::create_pipeline()
{
    if (!pipeline) {
        pipeline =
            cuda::create_optix_pipeline({ raygen_group, hg, miss_group });
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

std::tuple<float*, unsigned>
ScratchIntersectionContext::intersect_line_with_rays(
    float* lines,
    unsigned line_count,
    float* patches,
    unsigned patch_count,
    float width)
{
    auto vertex_count = line_count * 2;

    using namespace cuda;
    optix_init();
    std::string filename =
        RENDERER_SHADER_DIR + std::string("shaders/glints/glints.cu");

    this->line_end_vertices =
        borrow_cuda_linear_buffer({ vertex_count, 3 * sizeof(float) }, lines);

    create_raygen(filename);
    create_cylinder_intersection_shader();
    create_hitgroup_module(filename);
    create_hitgroup();
    create_miss_group(filename);
    create_pipeline();
    create_width_buffer(vertex_count, width);
    create_indices(vertex_count);

    handle = create_linear_curve_optix_traversable(
        { line_end_vertices->get_device_ptr() },
        vertex_count,
        { widths->get_device_ptr() },
        { indices->get_device_ptr() },
        line_count);

    int buffer_size = ratio * patch_count;

    if (buffer_size != _buffer_size) {
        append_buffer = AppendStructuredBuffer<uint2>(buffer_size);
    }
    this->_buffer_size = buffer_size;

    CUDALinearBufferDesc desc;
    desc.size = sizeof(Patch);
    desc.element_count = patch_count;

    patches_buffer = borrow_cuda_linear_buffer(desc, patches);

    glints_params =
        create_cuda_linear_buffer<GlintsTracingParams>(GlintsTracingParams{
            handle->getOptiXTraversable(),
            reinterpret_cast<Patch*>(patches_buffer->get_device_ptr()),
            append_buffer.get_device_queue_ptr() });

    optix_trace_ray<GlintsTracingParams>(
        handle, pipeline, glints_params->get_device_ptr(), patch_count, 1, 1);

    this->_vertex_count = vertex_count;
    this->primitive_count = line_count;
    this->patch_count = patch_count;

    return std::make_tuple(
        reinterpret_cast<float*>(append_buffer.get_underlying_buffer_ptr()),
        append_buffer.get_size());
}

void ScratchIntersectionContext::reset()
{
    raygen_group = nullptr;
    cylinder_module = nullptr;
    hg_module = nullptr;
    hg = nullptr;
    miss_group = nullptr;
    pipeline = nullptr;
    line_end_vertices = nullptr;
    widths = nullptr;
    indices = nullptr;
    handle = nullptr;
    patches_buffer = nullptr;
    glints_params = nullptr;
    _width = 0;
    _vertex_count = 0;
    ratio = 1.5f;
}

void ScratchIntersectionContext::set_max_pair_buffer_ratio(float ratio)
{
    this->ratio = ratio;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE