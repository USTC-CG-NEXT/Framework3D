
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(float_3dto2d)
{
    b.add_input<std::vector<std::vector<float>>>("Input 3D vector");
    b.add_output<std::vector<std::vector<float>>>("Output 2D vector");
    // Function content omitted
}

NODE_EXECUTION_FUNCTION(float_3dto2d)
{
    // Function content omitted
    auto input =
        params.get_input<std::vector<std::vector<float>>>("Input 3D vector");

    std::vector<std::vector<float>> output;
    output.clear();

    for (const auto& vec : input) {
        if (vec.size() >= 2) {
            output.push_back({ vec[0], vec[1] });
        }
    }
    params.set_output("Output 2D vector", output);
    return true;
}

NODE_DECLARATION_REQUIRED(float_3dto2d)
NODE_DECLARATION_UI(float_3dto2d);
NODE_DEF_CLOSE_SCOPE
