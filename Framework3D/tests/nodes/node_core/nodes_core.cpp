#include <gtest/gtest.h>

#include <entt/meta/meta.hpp>

#include "api.hpp"
#include "node.hpp"
#include "node_tree.hpp"

using namespace USTC_CG::nodes;
using namespace USTC_CG;

class NodeCoreTest : public ::testing::Test {
   protected:
    void TearDown() override
    {
        entt::meta_reset();
    }
};

TEST_F(NodeCoreTest, RegisterCppType)
{
    nodes::register_cpp_type<int>();
    entt::meta_reset();
}

TEST_F(NodeCoreTest, GetSocketType)
{
    std::unique_ptr<SocketType> socket_type = nodes::get_socket_type<int>();
    ASSERT_EQ(socket_type, nullptr);

    nodes::register_cpp_type<int>();

    socket_type = nodes::get_socket_type<int>();
    ASSERT_NE(socket_type, nullptr);
}

TEST_F(NodeCoreTest, CreateNodeTree)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();
    auto tree = nodes::create_node_tree(descriptor);
    ASSERT_NE(tree, nullptr);
}

TEST_F(NodeCoreTest, RegisterNode)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();
    std::unique_ptr<NodeTypeInfo> node_type_info =
        std::make_unique<NodeTypeInfo>("test_node");
    descriptor->register_node(std::move(node_type_info));
}

TEST_F(NodeCoreTest, CreateNode)
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

TEST_F(NodeCoreTest, NodeSocket)
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

TEST_F(NodeCoreTest, SerializeDeserialize)
{
    std::shared_ptr<NodeTreeDescriptor> descriptor =
        create_node_tree_descriptor();
    std::unique_ptr<NodeTypeInfo> node_type_info =
        std::make_unique<NodeTypeInfo>("test_node");

    nodes::register_cpp_type<int>();
    nodes::register_cpp_type<float>();
    nodes::register_cpp_type<std::string>();

    node_type_info->set_declare_function([](NodeDeclarationBuilder& b) {
        b.add_input<float>("test_socket").min(0).max(1).default_val(0);
        b.add_input<int>("test_socket2").min(-15).max(3).default_val(1);
        b.add_input<std::string>("string_socket").default_val("aaa");
    });

    descriptor->register_node(std::move(node_type_info));

    auto tree = nodes::create_node_tree(descriptor);
    auto node = tree->add_node("test_node");
    ASSERT_NE(node, nullptr);

    auto tree_serialize = tree->serialize(4);
    std::cout << tree_serialize << std::endl;

    auto tree2 = nodes::create_node_tree(descriptor);
    tree2->Deserialize(tree_serialize);

    auto serialize_2 = tree2->serialize(4);

    ASSERT_EQ(tree_serialize, serialize_2);
}
