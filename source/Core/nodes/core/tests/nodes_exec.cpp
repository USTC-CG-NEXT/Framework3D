#include <gtest/gtest.h>

#include <entt/meta/meta.hpp>

#include "nodes/core/api.hpp"
#include "nodes/core/node.hpp"
#include "nodes/core/node_tree.hpp"

using namespace USTC_CG;

class NodeExecTest : public ::testing::Test {
   protected:
    void SetUp() override
    {
        register_cpp_type<int>();
        register_cpp_type<float>();
        register_cpp_type<std::string>();

        std::shared_ptr<NodeTreeDescriptor> descriptor =
            std::make_shared<NodeTreeDescriptor>();

        // register adding node

        NodeTypeInfo add_node;
        add_node.id_name = "add";
        add_node.ui_name = "Add";
        add_node.ALWAYS_REQUIRED = true;
        add_node.set_declare_function([](NodeDeclarationBuilder& b) {
            b.add_input<int>("a");
            b.add_input<int>("b").default_val(1).min(0).max(10);
            b.add_output<int>("result");
        });

        add_node.set_execution_function([](ExeParams params) {
            auto a = params.get_input<int>("a");
            auto b = params.get_input<int>("b");
            params.set_output("result", a + b);
            return true;
        });

        descriptor->register_node(add_node);

        tree = create_node_tree(descriptor);
    }

    void TearDown() override
    {
        entt::meta_reset();
    }
    std::unique_ptr<NodeTree> tree;
};

 TEST_F(NodeExecTest, NodeExecSimple)
{
    NodeTreeExecutorDesc desc;
    desc.policy = NodeTreeExecutorDesc::Policy::Eager;
    auto executor = create_node_tree_executor(desc);

    auto add_node = tree->add_node("add");

    executor->prepare_tree(tree.get());

    auto a = add_node->get_input_socket("a");
    auto b = add_node->get_input_socket("b");
    executor->sync_node_from_external_storage(a, 1);
    executor->sync_node_from_external_storage(b, 2);

    executor->execute_tree(tree.get());

    entt::meta_any result;
    executor->sync_node_to_external_storage(
        add_node->get_output_socket("result"), result);

    // Type is int
    ASSERT_EQ(result.type().info().name(), "int");
    ASSERT_EQ(result.cast<int>(), 3);
}

 TEST_F(NodeExecTest, NodeExecWithLink)
{
    NodeTreeExecutorDesc desc;
    desc.policy = NodeTreeExecutorDesc::Policy::Eager;
    auto executor = create_node_tree_executor(desc);

    std::vector<Node*> add_nodes;

    for (int i = 0; i < 20; i++) {
        auto add_node = tree->add_node("add");
        add_nodes.push_back(add_node);
    }

    for (int i = 0; i < add_nodes.size() - 1; i++) {
        auto link = tree->add_link(
            add_nodes[i]->get_output_socket("result"),
            add_nodes[i + 1]->get_input_socket("a"));
    }

    executor->prepare_tree(tree.get());

    // Set the first node.a to 1

    auto a = add_nodes[0]->get_input_socket("a");
    executor->sync_node_from_external_storage(a, 1);

    // Set all the node.b to 2
    for (auto node : add_nodes) {
        auto b = node->get_input_socket("b");
        executor->sync_node_from_external_storage(b, 2);
    }

    executor->execute_tree(tree.get());

    // Get the last node result

    entt::meta_any result;
    executor->sync_node_to_external_storage(
        add_nodes.back()->get_output_socket("result"), result);

    // Type is int
    ASSERT_EQ(result.type().info().name(), "int");
    ASSERT_EQ(result.cast<int>(), 41);
}

TEST_F(NodeExecTest, NodeExecWithLinkAndNodeGroup)
{
    NodeTreeExecutorDesc desc;
    desc.policy = NodeTreeExecutorDesc::Policy::Eager;
    auto executor = create_node_tree_executor(desc);

    std::vector<Node*> add_nodes;
    std::vector<Node*> nodes_to_group;

    auto add_node_0 = tree->add_node("add");

    auto add_node_1 = tree->add_node("add");
    auto add_node_2 = tree->add_node("add");

    tree->add_link(
        add_node_0->get_output_socket("result"),
        add_node_1->get_input_socket("a"));
    tree->add_link(
        add_node_1->get_output_socket("result"),
        add_node_2->get_input_socket("a"));

    tree->group_up({ add_node_1 });

    auto input_0_a = add_node_0->get_input_socket("a");
    auto input_0_b = add_node_0->get_input_socket("b");

    executor->prepare_tree(tree.get());

    executor->sync_node_from_external_storage(input_0_a, 1);
    executor->sync_node_from_external_storage(input_0_b, 2);

    executor->execute_tree(tree.get());

    entt::meta_any value_out = 0;

    executor->sync_node_to_external_storage(
        add_node_2->get_output_socket("result"), value_out);

    std::cout << value_out.cast<int>() << std::endl;
}
