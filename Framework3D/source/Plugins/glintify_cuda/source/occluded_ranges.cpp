#include <glintify/stroke.h>

#include <RHI/internal/cuda_extension.hpp>

#include "glintify/glintify_params.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace stroke {
void calc_planar_ranges_with_occlusion(
    const cuda::CUDALinearBufferHandle& d_strokes,
    const std::vector<glm::vec3>& occlusion_vertices,
    const std::vector<unsigned>& occlusion_indices,
    glm::vec3 world_camera_position,
    glm::vec2 camera_move_range)
{
    std::once_flag flag;
    std::call_once(flag, []() {
        cuda::optix_init();
        cuda::add_extra_relative_include_dir_for_optix(
            "../../source/Plugins/glintify_cuda/include");
    });

    auto device_vertices = cuda::create_cuda_linear_buffer(occlusion_vertices);
    auto device_indices = cuda::create_cuda_linear_buffer(occlusion_indices);

    auto triangular_occlusion_optix_accel_handle =
        cuda::create_mesh_optix_traversable(
            { device_vertices->get_device_ptr() },
            occlusion_vertices.size(),
            sizeof(glm::vec3),
            device_indices->get_device_ptr(),
            occlusion_indices.size(),
            false);

    auto raygen = cuda::create_optix_raygen(
        RENDERER_SHADER_DIR + std::string("shaders/glints/glintify.cu"),
        RGS_STR(mesh_glintify),
        "GlintifyParams");

    auto miss = cuda::create_optix_miss(
        RENDERER_SHADER_DIR + std::string("shaders/glints/glintify.cu"),
        MISS_STR(mesh_glintify),
        "GlintifyParams");

    auto hg_module = cuda::create_optix_module(
        RENDERER_SHADER_DIR + std::string("shaders/glints/glintify.cu"),
        "GlintifyParams");

    cuda::OptiXProgramGroupDesc hg_desc;
    hg_desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_HITGROUP)
        .set_entry_name(nullptr, nullptr, CHS_STR(mesh_glintify));
    auto hit_group = cuda::create_optix_program_group(
        hg_desc, { nullptr, nullptr, hg_module });

    auto pipeline = cuda::create_optix_pipeline(
        { raygen, hit_group, miss }, "GlintifyParams");

    auto glints_params =
        cuda::create_cuda_linear_buffer<GlintifyParams>(GlintifyParams{
            triangular_occlusion_optix_accel_handle->getOptiXTraversable(),
            reinterpret_cast<Stroke*>(d_strokes->get_device_ptr()) });

    auto stroke_count = d_strokes->getDesc().element_count;
    cuda::optix_trace_ray<GlintifyParams>(
        triangular_occlusion_optix_accel_handle,
        pipeline,
        glints_params->get_device_ptr(),
        stroke_count,
        1,
        1);
}
}  // namespace stroke

USTC_CG_NAMESPACE_CLOSE_SCOPE