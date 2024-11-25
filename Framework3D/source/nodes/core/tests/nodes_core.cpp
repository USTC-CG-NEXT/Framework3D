#include <gtest/gtest.h>

#include <entt/meta/meta.hpp>

#include "nodes/core/api.hpp"
#include "nodes/core/node.hpp"
#include "nodes/core/node_tree.hpp"

using namespace USTC_CG;

class NodeCoreTest : public ::testing::Test {
   protected:
    void SetUp() override
    {
        log::SetMinSeverity(Severity::Info);
        log::EnableOutputToConsole(true);
    }
    void TearDown() override
    {
        unregister_cpp_type();
    }
};

TEST_F(NodeCoreTest, TYPENAME)
{
    auto type = get_socket_type<int>();
    ASSERT_TRUE(type);

    type = get_socket_type(entt::hashed_string{ type_name<int>().data() });

    ASSERT_TRUE(type);

    // std::string
    register_cpp_type<std::string>();
    auto type2 =
        get_socket_type(entt::hashed_string{ type_name<std::string>().data() });
    ASSERT_TRUE(type2);

    // false
    auto type3 = get_socket_type(entt::hashed_string{ type_name<float>().data() });
    ASSERT_FALSE(type3);

    ASSERT_EQ(
        entt::hashed_string{ type_name<float>().data() }, entt::type_hash<float>());
}

TEST_F(NodeCoreTest, RegisterCppType)
{
    entt::meta_reset();
}

TEST_F(NodeCoreTest, CreateNodeTree)
{
    NodeTreeDescriptor descriptor;
    auto tree = create_node_tree(descriptor);
    ASSERT_NE(tree, nullptr);
}

TEST_F(NodeCoreTest, RegisterNode)
{
    NodeTreeDescriptor descriptor;
    NodeTypeInfo node_type_info("test_node");
    descriptor.register_node(node_type_info);
}

TEST_F(NodeCoreTest, CreateNode)
{
    NodeTreeDescriptor descriptor;
    NodeTypeInfo node_type_info("test_node");

    descriptor.register_node(node_type_info);

    auto tree = create_node_tree(descriptor);
    auto node = tree->add_node("test_node");
    ASSERT_NE(node, nullptr);

    auto node2 = tree->add_node("test_node");
    ASSERT_EQ(node2->ID, NodeId(2u));
}

TEST_F(NodeCoreTest, NodeSocket)
{
    NodeTreeDescriptor descriptor;

    NodeTypeInfo node_type_info("test_node");

    node_type_info.set_declare_function(
        [](NodeDeclarationBuilder& b) { b.add_input<int>("test_socket"); });

    // Don't allow unregistered types
    node_type_info.set_declare_function(
        [](NodeDeclarationBuilder& b) { b.add_input<float>("test_socket2"); });

    descriptor.register_node(std::move(node_type_info));

    auto tree = create_node_tree(descriptor);
    auto node = tree->add_node("test_node");
    ASSERT_NE(node, nullptr);
}

TEST_F(NodeCoreTest, NodeSocketWithSameName)
{
    NodeTreeDescriptor descriptor;

    NodeTypeInfo node_type_info("test_node");

    ASSERT_THROW(
        node_type_info.set_declare_function([](NodeDeclarationBuilder& b) {
            b.add_input<int>("test_socket");
            b.add_input<int>("test_socket");
        });
        , std::runtime_error);

    descriptor.register_node(std::move(node_type_info));

    auto tree = create_node_tree(descriptor);
}

TEST_F(NodeCoreTest, NodeLink)
{
    NodeTreeDescriptor descriptor;
    NodeTypeInfo node_type_info("test_node");

    node_type_info.set_declare_function([](NodeDeclarationBuilder& b) {
        b.add_input<int>("test_input");
        b.add_output<int>("test_output");
    });

    descriptor.register_node(std::move(node_type_info));

    auto tree = create_node_tree(descriptor);

    auto node = tree->add_node("test_node");
    auto node2 = tree->add_node("test_node");

    auto link = tree->add_link(
        node->get_output_socket("test_output")->ID,
        node2->get_input_socket("test_input")->ID);

    ASSERT_NE(link, nullptr);

    tree->delete_link(link);

    link = tree->add_link(
        node->get_output_socket("test_output"),
        node2->get_input_socket("test_input"));

    ASSERT_NE(link, nullptr);

    // Re adding link is not acceptable
    ASSERT_THROW(tree->add_link(
        node->get_output_socket("test_output"),
        node2->get_input_socket("test_input"));
                 , std::runtime_error);

    // Link count

    ASSERT_EQ(tree->links.size(), 1);
}

TEST_F(NodeCoreTest, NodeLinkConversion)
{
    NodeTreeDescriptor descriptor;
    NodeTypeInfo node_type_info("test_node");

    register_cpp_type<float>();

    node_type_info.set_declare_function([](NodeDeclarationBuilder& b) {
        b.add_input<int>("test_input");
        b.add_output<float>("test_output");
    });

    descriptor.register_node(std::move(node_type_info));

    descriptor.register_conversion<float, int>([](const float& from, int& to) {
        to = from;
        return true;
    });

    auto tree = create_node_tree(descriptor);

    auto node = tree->add_node("test_node");
    auto node2 = tree->add_node("test_node");

    auto link = tree->add_link(
        node->get_output_socket("test_output")->ID,
        node2->get_input_socket("test_input")->ID);

    tree->delete_link(link);

    link = tree->add_link(
        node->get_output_socket("test_output"),
        node2->get_input_socket("test_input"));

    ASSERT_NE(link, nullptr);

    // Re adding link is not acceptable
    ASSERT_THROW(tree->add_link(
        node->get_output_socket("test_output"),
        node2->get_input_socket("test_input"));
                 , std::runtime_error);
}

TEST_F(NodeCoreTest, NodeRemove)
{
    NodeTreeDescriptor descriptor;
    NodeTypeInfo node_type_info("test_node");

    node_type_info.set_declare_function([](NodeDeclarationBuilder& b) {
        b.add_input<int>("test_input");
        b.add_output<int>("test_output");
    });

    descriptor.register_node(std::move(node_type_info));

    auto tree = create_node_tree(descriptor);

    auto node = tree->add_node("test_node");
    auto node2 = tree->add_node("test_node");

    auto link = tree->add_link(
        node->get_output_socket("test_output"),
        node2->get_input_socket("test_input"));

    tree->delete_node(node);

    ASSERT_EQ(tree->nodes.size(), 1);
    ASSERT_EQ(tree->links.size(), 0);
}

TEST_F(NodeCoreTest, PressureTestAddRemove)
{
    NodeTreeDescriptor descriptor;
    NodeTypeInfo node_type_info("test_node");

    register_cpp_type<float>();

    node_type_info.set_declare_function([](NodeDeclarationBuilder& b) {
        b.add_input<int>("test_input");
        b.add_output<int>("test_output");
    });

    descriptor.register_node(std::move(node_type_info));
    descriptor.register_conversion<float, int>([](const float& from, int& to) {
        to = from;
        return true;
    });

    auto tree = create_node_tree(descriptor);

    // Randomly add nodes, also links, to a tree style

    Node* previous_leaf = nullptr;
    for (int i = 0; i < 20; ++i) {
        Node* node = tree->add_node("test_node");
        auto node2 = tree->add_node("test_node");
        auto node3 = tree->add_node("test_node");

        auto link1 = tree->add_link(
            node->get_output_socket("test_output"),
            node2->get_input_socket("test_input"));

        auto link2 = tree->add_link(
            node->get_output_socket("test_output"),
            node3->get_input_socket("test_input"));

        if (previous_leaf) {
            tree->add_link(
                previous_leaf->get_output_socket("test_output"),
                node->get_input_socket("test_input"));
        }

        previous_leaf = node;
    }

    // Remove all nodes
    for (int i = 0; i < 60; ++i) {
        tree->delete_node(tree->nodes[0].get());
    }

    ASSERT_EQ(tree->nodes.size(), 0);
    ASSERT_EQ(tree->links.size(), 0);
    ASSERT_EQ(tree->has_available_link_cycle, false);
}

TEST_F(NodeCoreTest, SerializeDeserialize)
{
    NodeTreeDescriptor descriptor;
    NodeTypeInfo node_type_info("test_node");

    register_cpp_type<float>();
    register_cpp_type<std::string>();

    node_type_info.set_declare_function([](NodeDeclarationBuilder& b) {
        b.add_input<float>("test_socket").min(0).max(1).default_val(0);
        b.add_input<int>("test_socket2").min(-15).max(3).default_val(1);
        b.add_input<std::string>("string_socket").default_val("aaa");
        b.add_output<int>("output");
    });

    descriptor.register_node(std::move(node_type_info));

    auto tree = create_node_tree(descriptor);
    auto node = tree->add_node("test_node");
    ASSERT_NE(node, nullptr);

    auto tree_serialize = tree->serialize(4);

    auto tree2 = create_node_tree(descriptor);
    tree2->Deserialize(tree_serialize);

    auto serialize_2 = tree2->serialize(4);

    ASSERT_EQ(tree_serialize, serialize_2);

    // Test with links
    auto node2 = tree->add_node("test_node");
    auto node3 = tree->add_node("test_node");
    auto link = tree->add_link(
        node->get_output_socket("output"),
        node2->get_input_socket("test_socket2"));

    // Deserialize with a newly defined tree, with one socket removed
}
