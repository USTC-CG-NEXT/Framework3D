#include "NODES_FILES_DIR.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "RCore/Backend.hpp"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "resource_allocator_instance.hpp"


namespace USTC_CG::node_discuss {
static void node_declare(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Geometry>("Input");
    b.add_output<decl::Float1Buffer>("Output");
}

static void node_exec(ExeParams params)
{

}

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "discuss");
    strcpy(ntype.id_name, "node_discuss");

    render_node_type_base(&ntype);
    ntype.node_execute = node_exec;
    ntype.declare = node_declare;
    nodeRegisterType(&ntype);
}

NOD_REGISTER_NODE(node_register)
}  // namespace USTC_CG::node_discuss
