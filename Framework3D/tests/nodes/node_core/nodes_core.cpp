#include <gtest/gtest.h>

#include <fstream>

#include "api.hpp"
#include "node.hpp"
#include "node_tree.hpp"
#include "nodes.hpp"
#include "socket_types/basic_socket_types.hpp"

using namespace USTC_CG::nodes;
using namespace USTC_CG;

TEST(cpp, register_cpp_type)
{
    nodes::register_cpp_type<int>();
    entt::meta_reset();
}

TEST(socket, get_socket_type)
{
    // Before registering the type, the socket should be nullptr
    std::unique_ptr<SocketType> socket_type = nodes::get_socket_type<int>();
    ASSERT_EQ(socket_type, nullptr);

    nodes::register_cpp_type<int>();

    socket_type = nodes::get_socket_type<int>();
    ASSERT_NE(socket_type, nullptr);

    entt::meta_reset();
}

// Test create tree
TEST(tree, create_node_tree)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();

    auto tree = nodes::create_node_tree(descriptor);
    ASSERT_NE(tree, nullptr);
}

TEST(node, register_node)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();

    std::unique_ptr<NodeTypeInfo> node_type_info =
        std::make_unique<NodeTypeInfo>("test_node");

    descriptor->register_node(std::move(node_type_info));
}

TEST(node, create_node)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();

    std::unique_ptr<NodeTypeInfo> node_type_info =
        std::make_unique<NodeTypeInfo>("test_node");

    descriptor->register_node(std::move(node_type_info));

    auto tree = nodes::create_node_tree(descriptor);

    auto node = tree->add_node("test_node");
    ASSERT_NE(node, nullptr);

    auto node2 = tree->add_node("test_node");
    ASSERT_EQ(node2->ID, NodeId(1u));
}

TEST(socket, node_socket)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();

    std::unique_ptr<NodeTypeInfo> node_type_info =
        std::make_unique<NodeTypeInfo>("test_node");

    nodes::register_cpp_type<int>();

    node_type_info->set_declare_function(
        [](NodeDeclarationBuilder& b) { b.add_input<int>("test_socket"); });

    descriptor->register_node(std::move(node_type_info));

    auto tree = nodes::create_node_tree(descriptor);

    auto node = tree->add_node("test_node");
    ASSERT_NE(node, nullptr);
}

TEST(io, serialize)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();

    std::unique_ptr<NodeTypeInfo> node_type_info =
        std::make_unique<NodeTypeInfo>("test_node");

    nodes::register_cpp_type<float>();

    node_type_info->set_declare_function(
        [](NodeDeclarationBuilder& b) { b.add_input<int>("test_socket"); });

    descriptor->register_node(std::move(node_type_info));

    auto tree = nodes::create_node_tree(descriptor);

    auto node = tree->add_node("test_node");
    ASSERT_NE(node, nullptr);

    nlohmann::json value;
    node->serialize(value);

    std::cout << value.dump(4);
}