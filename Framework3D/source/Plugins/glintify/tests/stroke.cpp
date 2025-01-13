#include "../source/stroke.h"

#include <GUI/widget.h>
#include <GUI/window.h>
#include <gtest/gtest.h>

#include "glintify/glintify.hpp"
#include "../source/stroke.h"

using namespace USTC_CG;

TEST(StrokeSystem, get_all_endpoints)
{
    StrokeSystem stroke_system;
    stroke_system.add_virtual_point(glm::vec3(0, 0, 0));
    stroke_system.calc_scratches();

    auto endpoints = stroke_system.get_all_endpoints();
    ASSERT_FALSE(endpoints.empty());
    ASSERT_EQ(endpoints.size(), 128);
    ASSERT_EQ(endpoints[0].size(), 16);
}


