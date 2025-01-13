#include <GUI/widget.h>
#include <GUI/window.h>
#include <gtest/gtest.h>

#include "glintify/glintify.hpp"

using namespace USTC_CG;

TEST(StrokeSystem, get_all_endpoints)
{
    StrokeSystem stroke_system;

    stroke_system.calc_scratches();

    auto endpoints = stroke_system.get_all_endpoints();

    assert(!endpoints.empty());
}
