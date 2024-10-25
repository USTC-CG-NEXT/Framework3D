#include "api.hpp"

#include "node_tree.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
std::shared_ptr<NodeTreeDescriptor> nodes::default_node_tree_descriptor()
{
    auto descriptor = std::make_shared<NodeTreeDescriptor>();

    return descriptor;
}

std::unique_ptr<NodeTree> nodes::create_node_tree(
    std::shared_ptr<NodeTreeDescriptor> descriptor)
{
    return std::make_unique<NodeTree>(descriptor);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE