#include <glintify/glintify.hpp>

#include "stroke.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace stroke;
std::vector<std::vector<glm::vec2>> StrokeSystem::get_all_endpoints()
{
    std::vector<std::vector<glm::vec2>> endpoints;

    for (int i = 0; i < strokes.size(); ++i) {
        auto device_stroke = strokes[i];
        Stroke stroke = device_stroke->get_host_value<Stroke>();

        for (int j = 0; j < stroke.scratch_count; ++j) {
            std::vector<glm::vec2> stroke_endpoints;
            for (int k = 0; k < SAMPLE_POINT_COUNT; ++k) {
                stroke_endpoints.push_back(stroke.scratches[j].sample_point[k]);
            }

            endpoints.push_back(stroke_endpoints);
        }
    }

    return endpoints;
}

void StrokeSystem::calc_scratches()
{
    std::vector<Stroke*> stroke_addrs;

    for (const auto& stroke : strokes) {
        stroke_addrs.push_back(
            reinterpret_cast<Stroke*>(stroke->get_device_ptr()));
    }

    auto d_strokes = cuda::create_cuda_linear_buffer(stroke_addrs);

    stroke::calc_scratches(
        d_strokes, world_camera_position, world_light_position);
}

void StrokeSystem::add_virtual_point(const glm::vec3& vec)
{
    Stroke stroke;
    stroke.set_virtual_point_position(vec);

    strokes.push_back(cuda::create_cuda_linear_buffer(stroke));
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
