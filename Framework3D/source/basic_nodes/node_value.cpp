#include "basic_node_base.h"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(int)
{
    b.add_input<int>("value").min(0).max(10).default_val(1);
    b.add_output<int>("value");
}

NODE_EXECUTION_FUNCTION(int)
{
    auto val = params.get_input<int>("value");
    params.set_output("value", val);
    return true;
}

NODE_DECLARATION_FUNCTION(float)
{
    b.add_input<float>("value").min(0).max(10).default_val(1);
    b.add_output<float>("value");
}

NODE_EXECUTION_FUNCTION(float)
{
    auto val = params.get_input<float>("value");
    params.set_output("value", val);
    return true;
}

NODE_DECLARATION_FUNCTION(bool)
{
    b.add_input<bool>("value").default_val(true);
    b.add_output<bool>("value");
}

NODE_EXECUTION_FUNCTION(bool)
{
    auto val = params.get_input<bool>("value");
    params.set_output("value", val);
    return true;
}

NODE_DECLARATION_UI(value);
NODE_DEF_CLOSE_SCOPE
