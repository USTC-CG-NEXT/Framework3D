#include "nodes/core/def/node_def.hpp"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/vt/array.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_vertex_uv)
{
    b.add_input<int>("Size").min(1).max(441).default_val(441);

    b.add_output<pxr::VtArray<pxr::GfVec2f>>("vertex_uv");
}

NODE_EXECUTION_FUNCTION(test_vertex_uv)
{
    auto size = params.get_input<int>("Size");

    pxr::VtArray<pxr::GfVec2f> vertex_uv(size);

    // 在正方形上均匀采样
    for (int i = 0; i < size; ++i) {
        float x = i % 21;
        float y = i / 21;
        vertex_uv[i] = { x / 20, y / 20 };
    }

    params.set_output("vertex_uv", vertex_uv);

    return true;
}

NODE_DEF_CLOSE_SCOPE