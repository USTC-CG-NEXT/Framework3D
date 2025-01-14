#include <glintify/glintify.hpp>

#include "Logger/Logger.h"
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
        stroke::calc_planar_ranges_with_occlusion(
            d_strokes,
            occlusion_vertices,
            occlusion_indices,
            world_camera_position,
            camera_move_range);
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

    fill_ranges();

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
