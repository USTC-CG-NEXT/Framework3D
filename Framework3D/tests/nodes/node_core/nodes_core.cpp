#include <gtest/gtest.h>

#include "api.hpp"
#include "node_tree.hpp"

using namespace USTC_CG::nodes;
using namespace USTC_CG;

TEST(registry, register_cpp_type)
{
    nodes::register_cpp_type<int>();
    entt::meta_reset();
}

TEST(registry, register_socket_type)
{
    // Before registering the type, the socket should be nullptr
    std::unique_ptr<SocketType> socket_type = nodes::get_socket_type<int>();
    ASSERT_EQ(socket_type, nullptr);

    nodes::register_cpp_type<int>();

    socket_type = nodes::get_socket_type<int>();
    ASSERT_NE(socket_type, nullptr);

    entt::meta_reset();
}

TEST(making, make_socket)
{
    nodes::register_cpp_type<int>();
    auto socket_type = nodes::get_socket_type<int>();

    auto socket = nodes::make_socket(socket_type);
    ASSERT_NE(socket, nullptr);
}

// Test create tree
TEST(creation, create_node_tree)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();
    auto tree = nodes::create_node_tree(descriptor);
    ASSERT_NE(tree, nullptr);
}
