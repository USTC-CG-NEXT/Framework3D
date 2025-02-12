#include <functional>
#include <random>

#include "nodes/core/def/node_def.hpp"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/vt/array.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_2d_function)
{
    b.add_input<int>("Seed").min(0).max(10).default_val(0);
    b.add_input<int>("Vertices Counts").min(3).max(10).default_val(4);
    b.add_output<pxr::VtArray<pxr::GfVec2f>>("2d_vertex");
    b.add_output<std::function<float(float, float)>>("function");
}

NODE_EXECUTION_FUNCTION(test_2d_function)
{
    auto seed = params.get_input<int>("Seed");
    auto vertices_counts = params.get_input<int>("Vertices Counts");

    // 生成随机顶点位置，顶点个数为vertices_counts，范围为[0, 1]
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    pxr::VtArray<pxr::GfVec2f> vertices(vertices_counts);
    for (int i = 0; i < vertices_counts; ++i) {
        vertices[i] = { dis(gen), dis(gen) };
    }

    params.set_output("2d_vertex", vertices);

    // 输出一个二维标量函数，返回值为sqrt(x^2 + y^2)
    std::function<float(float, float)> function = [](float x, float y) {
        return sqrt(x * x + y * y);
    };
    params.set_output("function", function);

    return true;
}

NODE_DEF_CLOSE_SCOPE