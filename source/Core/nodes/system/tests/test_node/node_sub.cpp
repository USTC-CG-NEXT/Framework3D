#include <iostream>
#include <nodes/core/def/node_def.hpp>
NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(sub)
{
    b.add_input<int>("value").min(0).max(10).default_val(1);
    b.add_input<float>("float").min(0).max(10).default_val(1);

    b.add_output<int>("value");
}

NODE_EXECUTION_FUNCTION(sub)
{
    auto val = params.get_input<int>("value");
    params.set_output("value", val);
    return true;
}
NODE_DEF_CLOSE_SCOPE