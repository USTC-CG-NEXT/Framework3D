#include <random>

#include "nodes/core/def/node_def.hpp"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/vt/array.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_random_color)
{
    b.add_input<int>("Seed").min(0).max(10).default_val(0);
    b.add_input<int>("Size").min(1).max(10).default_val(4);

    b.add_output<pxr::VtArray<pxr::GfVec3f>>("Color");
}

NODE_EXECUTION_FUNCTION(test_random_color)
{
    auto seed = params.get_input<int>("Seed");
    auto size = params.get_input<int>("Size");

    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    pxr::VtArray<pxr::GfVec3f> colors(size);

    for (int i = 0; i < size; ++i) {
        colors[i] = { dis(gen), dis(gen), dis(gen) };
    }

    params.set_output("Color", colors);

    return true;
}

NODE_DECLARATION_UI(test_random_color);

NODE_DEF_CLOSE_SCOPE