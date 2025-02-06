#include <iostream>
#include <nodes/core/def/node_def.hpp>
#include <nodes/core/node_exec.hpp>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_UI(add)
{
    return "Add";
}

NODE_DECLARATION_FUNCTION(add)
{
    b.add_input<int>("value").min(0).max(10).default_val(1);
    b.add_input<int>("value2").min(0).max(10).default_val(1);
    b.add_output<int>("value");

    b.add_input_group<int>("input_group").set_runtime_dynamic(true);
    b.add_output_group("output_group").set_runtime_dynamic(true);
}

NODE_EXECUTION_FUNCTION(add)
{
    auto val = params.get_input<int>("value");
    auto val2 = params.get_input<int>("value2");
    auto sum = val + val2;
    params.set_output("value", sum);
    return true;
}

NODE_DEF_CLOSE_SCOPE