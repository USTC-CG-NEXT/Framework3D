#include <iostream>
#include <nodes/core/def/node_def.hpp>
#include <nodes/core/node_exec.hpp>

NODE_DEF_OPEN_SCOPE
__declspec(dllexport) const char* node_ui_name()
{
    return "Add";
}

NODE_DECLARATION_FUNCTION(add)
{
    b.add_input<int>("value").min(0).max(10).default_val(1);
    b.add_output<int>("value");
}

NODE_EXECUTION_FUNCTION(add)
{
    auto val = params.get_input<int>("value");

    params.set_output("value", val);
    return true;
}

NODE_DEF_CLOSE_SCOPE