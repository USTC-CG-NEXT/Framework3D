#include "glintify/stroke.h"

#include <GUI/widget.h>
#include <GUI/window.h>
#include <gtest/gtest.h>

#include <glintify/glintify.hpp>

#include "glintify/mesh.hpp"

using namespace USTC_CG;

TEST(StrokeSystem, fill_ranges)
{
    StrokeSystem stroke_system;
    stroke_system.set_camera_position(glm::vec3(0, 0, -3));
    stroke_system.set_camera_move_range(glm::vec2(-1., 1.));
    stroke_system.add_virtual_point(glm::vec3(-0.0, 0.1, -1));

    stroke_system.fill_ranges();
}

TEST(StrokeSystem, fill_ranges_occluded)
{
    StrokeSystem stroke_system;
    stroke_system.set_camera_position(glm::vec3(0, 0, -3));
    stroke_system.set_camera_move_range(glm::vec2(-1., 1.));

    Mesh mesh = Mesh::load_from_obj("cube.obj");
    stroke_system.set_occlusion(mesh.vertices, mesh.indices);

    auto sampled_points = mesh.sample_on_edges(0.099);

    for (auto& point : sampled_points) {
        stroke_system.add_virtual_point(point);
    }

    stroke_system.fill_ranges(true);
}

TEST(StrokeSystem, calc_scratch)
{
    stroke::Stroke s;

    s.virtual_point_position = glm::vec3(-0.5, 0, -1);

    s.range[0] = std::make_pair(glm::vec2(0.25, 0.4), glm::vec2(0.75, 0.4));

    s.calc_scratch(1, glm::vec3(-1, 3, -3));
}
