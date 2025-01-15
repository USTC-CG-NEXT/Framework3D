#include <glintify/glintify.hpp>

#include "Logger/Logger.h"
#include "glintify/glintify_params.h"
#include "glintify/stroke.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace stroke;
std::vector<std::vector<glm::vec2>> StrokeSystem::get_all_endpoints()
{
    if (is_dirty) {
        endpoints_cache.clear();

        for (int i = 0; i < strokes.size(); ++i) {
            auto device_stroke = strokes[i];
            Stroke stroke = device_stroke->get_host_value<Stroke>();

            for (int j = 0; j < stroke.scratch_count; ++j) {
                std::vector<glm::vec2> stroke_endpoints;
                for (int k = 0; k < stroke.scratches[j].valid_sample_count;
                     ++k) {
                    if (stroke.scratches[j].should_begin_new_line_mask[k] &&
                        !stroke_endpoints.empty()) {
                        if (stroke_endpoints.size() > 1) {
                            endpoints_cache.push_back(stroke_endpoints);
                        }
                        stroke_endpoints.clear();
                    }
                    stroke_endpoints.push_back(
                        stroke.scratches[j].sample_point[k]);
                }
                if (stroke_endpoints.size() > 1) {
                    endpoints_cache.push_back(stroke_endpoints);
                }
            }
        }
        is_dirty = false;
    }

    return endpoints_cache;
}

void StrokeSystem::prepare_occlusion_test_pipeline()
{
    raygen = cuda::create_optix_raygen(
        RENDERER_SHADER_DIR + std::string("shaders/glints/glintify.cu"),
        RGS_STR(mesh_glintify),
        "params");

    miss = cuda::create_optix_miss(
        RENDERER_SHADER_DIR + std::string("shaders/glints/glintify.cu"),
        MISS_STR(mesh_glintify),
        "params");

    hg_module = cuda::create_optix_module(
        RENDERER_SHADER_DIR + std::string("shaders/glints/glintify.cu"),
        "params");

    cuda::OptiXProgramGroupDesc hg_desc;
    hg_desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_HITGROUP)
        .set_entry_name(nullptr, nullptr, CHS_STR(mesh_glintify));
    hit_group = cuda::create_optix_program_group(
        hg_desc, { nullptr, nullptr, hg_module });

    pipeline =
        cuda::create_optix_pipeline({ raygen, hit_group, miss }, "params");
}

void StrokeSystem::fill_ranges(bool consider_occlusion)
{
    std::vector<Stroke*> stroke_addrs;

    for (const auto& stroke : strokes) {
        stroke_addrs.push_back(
            reinterpret_cast<Stroke*>(stroke->get_device_ptr()));
    }

    auto d_strokes = cuda::create_cuda_linear_buffer(stroke_addrs);

    if (!consider_occlusion) {
        if (on_plane_board) {
            stroke::calc_simple_plane_projected_ranges(
                d_strokes, world_camera_position, camera_move_range);
        }
    }
    else {
        std::call_once(optix_init_flag, []() {
            cuda::optix_init();
            cuda::add_extra_relative_include_dir_for_optix(
                "../../source/Plugins/glintify_cuda/include");
        });

        auto device_vertices =
            cuda::create_cuda_linear_buffer(occlusion_vertices);
        auto device_indices =
            cuda::create_cuda_linear_buffer(occlusion_indices);

        auto triangular_occlusion_optix_accel_handle =
            cuda::create_mesh_optix_traversable(
                { device_vertices->get_device_ptr() },
                occlusion_vertices.size(),
                sizeof(glm::vec3),
                device_indices->get_device_ptr(),
                occlusion_indices.size() / 3,
                false);

        if (!pipeline)
            prepare_occlusion_test_pipeline();

        auto glints_params =
            cuda::create_cuda_linear_buffer<GlintifyParams>(GlintifyParams{
                triangular_occlusion_optix_accel_handle->getOptiXTraversable(),
                reinterpret_cast<Stroke**>(d_strokes->get_device_ptr()),
                world_camera_position,
                camera_move_range });

        auto stroke_count = d_strokes->getDesc().element_count;
        cuda::optix_trace_ray<GlintifyParams>(
            triangular_occlusion_optix_accel_handle,
            pipeline,
            glints_params->get_device_ptr(),
            stroke_count,
            1,
            1);
    }
}

void StrokeSystem::set_occlusion(
    const std::vector<glm::vec3>& vertices,
    const std::vector<unsigned int>& indices)
{
    this->occlusion_vertices = vertices;
    this->occlusion_indices = indices;
}

void StrokeSystem::calc_scratches()
{
    std::vector<Stroke*> stroke_addrs;

    for (const auto& stroke : strokes) {
        stroke_addrs.push_back(
            reinterpret_cast<Stroke*>(stroke->get_device_ptr()));
    }

    auto d_strokes = cuda::create_cuda_linear_buffer(stroke_addrs);

    // fill_ranges();

    stroke::calc_scratches(
        d_strokes, world_camera_position, world_light_position);
}

void StrokeSystem::add_virtual_point(const glm::vec3& vec)
{
    Stroke stroke;
    stroke.set_virtual_point_position(vec);

    strokes.push_back(cuda::create_cuda_linear_buffer(stroke));
    is_dirty = true;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
