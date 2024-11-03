﻿#include "NODES_FILES_DIR.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "RCore/Backend.hpp"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "resource_allocator_instance.hpp"
#include "his_header.h"

namespace USTC_CG::node_discuss {
static void node_declare(NodeDeclarationBuilder& b)
{
}

static void node_exec(ExeParams params)
{
    
}

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "Discussion");
    strcpy(ntype.id_name, "node_discuss");

    render_node_type_base(&ntype);
    ntype.node_execute = node_exec;
    ntype.declare = node_declare;
    nodeRegisterType(&ntype);
}


}  // namespace USTC_CG::node_discuss
