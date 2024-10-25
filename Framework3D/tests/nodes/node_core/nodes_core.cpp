#include <gtest/gtest.h>

#include "api.hpp"
#include "node_tree.hpp"

using namespace USTC_CG::nodes;
using namespace USTC_CG;

TEST(registry, register_cpp_type)
{
    nodes::register_cpp_type<int>();
}

TEST(registry, register_socket)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();
    nodes::register_cpp_type<int>();

    auto socket_descriptor = nodes::get_socket_type_info("int");
    ASSERT_NE(socket_descriptor, nullptr);
}

TEST(registry, register_node)
{
}

// Test create tree
TEST(creation, create_node_tree)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();
    auto tree = nodes::create_node_tree(descriptor);
    ASSERT_NE(tree, nullptr);
}
