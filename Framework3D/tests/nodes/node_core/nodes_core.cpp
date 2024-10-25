#include <gtest/gtest.h>

#include "api.hpp"
#include "node_tree.hpp"

using namespace USTC_CG::nodes;
using namespace USTC_CG;

// Test create tree
TEST(creation, create_node_tree)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        default_node_tree_descriptor();
    auto tree = nodes::create_node_tree(descriptor);
    ASSERT_NE(tree, nullptr);
}

TEST(register, register_cpp_type)
{
    nodes::register_cpp_type<int>();
}

void node_declare()
{
}

void node_execution()
{
}

TEST(register, register_node)
{
    // auto tree = nodes::create_node_tree();
    // auto node = tree->add_node("test");
    // ASSERT_NE(node, nullptr);
}
