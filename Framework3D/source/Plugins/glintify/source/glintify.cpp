#include <glintify/glintify.hpp>

#include "stroke.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
std::vector<glm::vec2> StrokeSystem::get_all_endpoints()
{
    std::vector<glm::vec2> endpoints;

    for (int i = 0; i < strokes.size(); ++i) {
        auto device_stroke = strokes[i];
        Stroke stroke = device_stroke->get_host_value<Stroke>();

        for (int j = 0; j < stroke.scratch_count; ++j) {
            for (int k = 0; k < SAMPLE_POINTS; ++k) {
                endpoints.push_back(stroke.scratches[j].begin[k]);
                endpoints.push_back(stroke.scratches[j].end[k]);
            }
        }
    }

    return endpoints;
}

void StrokeSystem::calc_scratches()
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
