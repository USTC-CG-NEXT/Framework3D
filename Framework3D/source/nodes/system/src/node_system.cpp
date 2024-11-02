#include "nodes/system/node_system.hpp"

#include "node_system_dl.hpp"

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

std::unique_ptr<NodeSystem> create_dynamic_loading_system()
{
    return std::make_unique<NodeDynamicLoadingSystem>();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE