
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(float_3dto2d)
{
    b.add_input<std::vector<std::vector<float>>>("Input 3D vector");
    b.add_input<float>("Y").min(-10).max(10).default_val(0);
    // Function content omitted
}

NODE_EXECUTION_FUNCTION(float_3dto2d)
{
    // Function content omitted
    return true;
}

NODE_DECLARATION_REQUIRED(float_3dto2d)
NODE_DECLARATION_UI(float_3dto2d);
NODE_DEF_CLOSE_SCOPE
