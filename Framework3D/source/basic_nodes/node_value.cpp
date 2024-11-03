#include "basic_node_base.h"
namespace USTC_CG::node_value {
static void node_declare_int(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Int>("value").min(0).max(10).default_val(1);
    b.add_output<decl::Int>("value");
}

static void node_exec_int(ExeParams params)
{
    auto val = params.get_input<int>("value");
    params.set_output("value", val);
}

static void node_declare_float(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Float>("value").min(0).max(10).default_val(1);
    b.add_output<decl::Float>("value");
}

static void node_exec_float(ExeParams params)
{
    auto val = params.get_input<float>("value");
    params.set_output("value", val);
}

static void node_declare_bool(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Bool>("value").min(0).max(1).default_val(1);
    b.add_output<decl::Bool>("value");
}

static void node_exec_bool(ExeParams params)
{
    auto val = params.get_input<bool>("value");
    params.set_output("value", val);
}

}  // namespace USTC_CG::node_value
