#include <algorithm>

#include "basic_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(func_color_map)
{
    b.add_input<float1Buffer>("Vals");

    b.add_output<float3Buffer>("Colors");
}

NODE_EXECUTION_FUNCTION(func_color_map)
{
    auto input = params.get_input<pxr::VtArray<float>>("Vals");

    if (input.empty()) {
        throw std::runtime_error("Func Color Map: Require input data");
    }

    auto minmax = std::minmax_element(input.begin(), input.end());
    float min = *minmax.first;
    float max = *minmax.second;

    pxr::VtArray<pxr::GfVec3f> colors(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        float normalizedValue;
        if (input[i] <= 0) {
            float minToZeroRange = 0 - min;
            normalizedValue = (input[i] - min) / minToZeroRange * 0.5;
        }
        else {
            float zeroToMaxRange = max - 0;
            normalizedValue = 0.5 + (input[i] / zeroToMaxRange) * 0.5;
        }

        pxr::GfVec3f color(1 - normalizedValue, normalizedValue, 0);
        if (normalizedValue <= 0.5) {
            color = pxr::GfVec3f(
                2 * (0.5 - normalizedValue),
                1 - 2 * (0.5 - normalizedValue),
                0);
        }
        else {
            color = pxr::GfVec3f(
                2 * (1 - normalizedValue), 2 * (normalizedValue - 0.5), 0);
        }
        colors[i] = color;
    }

    params.set_output("Colors", colors);
}

NODE_DECLARATION_UI(func_color_map);
NODE_DEF_CLOSE_SCOPE
