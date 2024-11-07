#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

CONVERSION_DECLARATION_FUNCTION(int, float)
{
    b.add_input<int>("int");
    b.add_output<float>("float");
}

CONVERSION_EXECUTION_FUNCTION(int, float)
{
    const int input = params.get_input<int>("int");
    params.set_output<float>("float", static_cast<float>(input));
    return true;
}

CONVERSION_FUNC_NAME(int, float);

NODE_DEF_CLOSE_SCOPE