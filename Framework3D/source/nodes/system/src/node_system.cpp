#include "nodes/system/node_system.hpp"

#include "nodes/system/node_system_dl.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
void NodeSystem::init()
{
    this->node_tree = create_node_tree(node_tree_descriptor());
}

void NodeSystem::set_node_tree_executor(
    std::unique_ptr<NodeTreeExecutor> executor)
{
    node_tree_executor = std::move(executor);
}

NodeSystem::~NodeSystem()
{
}

void NodeSystem::finalize()
{
    if (node_tree_executor) {
        node_tree_executor->finalize(node_tree.get());
    }
}

void NodeSystem::execute(bool is_ui_execution, Node* required_node) const
{
    if (is_ui_execution && !allow_ui_execution) {
        return;
    }
    if (node_tree_executor) {
        return node_tree_executor->execute(node_tree.get(), required_node);
    }
}

NodeTree* NodeSystem::get_node_tree() const
{
    return node_tree.get();
}

NodeTreeExecutor* NodeSystem::get_node_tree_executor() const
{
    return node_tree_executor.get();
}

std::shared_ptr<NodeSystem> create_dynamic_loading_system()
{
    return std::make_shared<NodeDynamicLoadingSystem>();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE