#include "../source/stroke.h"

#include <GUI/widget.h>
#include <GUI/window.h>
#include <gtest/gtest.h>

#include "../source/stroke.h"
#include "glintify/glintify.hpp"

using namespace USTC_CG;
//
// TEST(StrokeSystem, get_all_endpoints)
//{
//    StrokeSystem stroke_system;
//    stroke_system.add_virtual_point(glm::vec3(0, 0, 0));
//    stroke_system.calc_scratches();
//
//    auto endpoints = stroke_system.get_all_endpoints();
//    ASSERT_FALSE(endpoints.empty());
//    ASSERT_EQ(endpoints.size(), 128);
//    ASSERT_EQ(endpoints[0].size(), 16);
//}

TEST(StrokeSystem, fill_ranges)
{
    StrokeSystem stroke_system;
    stroke_system.set_camera_position(glm::vec3(0, 0, -3));
    stroke_system.set_camera_move_range(glm::vec2(-1., 1.));
    stroke_system.add_virtual_point(glm::vec3(-0.0, 0.1, -1));

    stroke_system.fill_ranges();

    auto stroke = stroke_system.strokes[0]->get_host_value<stroke::Stroke>();

    std::cout << stroke.range[0].first.x << " " << stroke.range[0].first.y
              << std::endl;
    std::cout << stroke.range[0].second.x << " " << stroke.range[0].second.y
              << std::endl;

    // s.virtual_point_position = glm::vec3(-0.5, 0, -1);
    // s.add_virtual_point(glm::vec3(-1, 3, -3));
}

TEST(StrokeSystem, calc_scratch)
{
    stroke::Stroke s;

    s.virtual_point_position = glm::vec3(-0.5, 0, -1);

    s.range[0] = std::make_pair(glm::vec2(0.25, 0.4), glm::vec2(0.75, 0.4));

    s.calc_scratch(1, glm::vec3(-1, 3, -3));
}