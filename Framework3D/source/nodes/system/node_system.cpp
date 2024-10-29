#include "nodes/system/node_system.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE

void NodeSystem::init()
{
    this->node_tree = create_node_tree(node_tree_descriptor());

    this->node_tree_executor =
        create_node_tree_executor(node_tree_executor_desc());
}

NodeSystem::~NodeSystem()
{
}

NodeTree* NodeSystem::get_node_tree() const
{
    return node_tree.get();
}

NodeTreeExecutor* NodeSystem::get_node_tree_executor() const
{
    return node_tree_executor.get();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE