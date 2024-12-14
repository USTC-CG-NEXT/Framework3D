#include "glints.hpp"

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
    if (!widths || width != _width || vertex_count != vertex_count) {
        widths = cuda::create_cuda_linear_buffer(std::vector{ width, width });
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

std::tuple<float*, unsigned>
ScratchIntersectionContext::intersect_line_with_rays(
    float* lines,
    unsigned line_count,
    float* patches,
    unsigned patch_count,
    float width)
{
    this->primitive_count = line_count;
    auto vertex_count = line_count * 2;
    this->patch_count = patch_count;

    using namespace cuda;
    optix_init();
    std::string filename = "glints/glints.cu";

    this->line_end_vertices = borrow_cuda_linear_buffer(
        { static_cast<int>(vertex_count), 3 * sizeof(float) }, lines);

    create_raygen(filename);
    create_cylinder_intersection_shader();
    create_hitgroup_module(filename);
    create_hitgroup();
    create_miss_group(filename);
    create_pipeline();
    create_width_buffer(vertex_count, width);
    create_indices(vertex_count);

    handle = create_optix_traversable(
        { line_end_vertices->get_device_ptr() },
        vertex_count,
        { widths->get_device_ptr() },
        { indices->get_device_ptr() },
        line_count);

    int buffer_size = ratio * line_count * patch_count;
    this->_buffer_size = buffer_size;

    // if (buffer_size != _buffer_size)
    {
        append_buffer = AppendStructuredBuffer<uint2>(buffer_size);
    }

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