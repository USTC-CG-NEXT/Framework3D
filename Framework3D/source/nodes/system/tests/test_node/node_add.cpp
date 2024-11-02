#include <iostream>
#include <nodes/core/def/node_def.hpp>
#include <nodes/core/node_exec.hpp>

extern "C" {
__declspec(dllexport) const char* node_ui_name()
{
    return "Add";
}

__declspec(dllexport) void node_declare(USTC_CG::NodeDeclarationBuilder& b)
{
    b.add_input<int>("value").min(0).max(10).default_val(1);
    b.add_output<int>("value");
}

__declspec(dllexport) void node_execution(USTC_CG::ExeParams params)
{
    auto val = params.get_input<int>("value");
    params.set_output("value", val);
}
}