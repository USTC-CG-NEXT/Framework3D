#include <Eigen/Eigen>
#include <iostream>

#include "nodes/core/api.hpp"
#include "nodes/core/node_exec.hpp"
#include "nodes/core/node_tree.hpp"
using namespace USTC_CG;

int main()
{
    register_cpp_type<int>();
    register_cpp_type<float>();
    register_cpp_type<std::string>();

    register_cpp_type<Eigen::VectorXd>();

    std::shared_ptr<NodeTreeDescriptor> descriptor =
        std::make_shared<NodeTreeDescriptor>();

    // register adding node

    NodeTypeInfo add_node;
    add_node.id_name = "add";
    add_node.ui_name = "Add";
    add_node.ALWAYS_REQUIRED = true;
    add_node.set_declare_function([](NodeDeclarationBuilder& b) {
        b.add_input<Eigen::VectorXd>("a");
        b.add_input<Eigen::VectorXd>("b");
        b.add_output<Eigen::VectorXd>("result");
    });

    add_node.set_execution_function([](ExeParams params) {
        auto a = params.get_input<Eigen::VectorXd>("a");
        auto b = params.get_input<Eigen::VectorXd>("b");
        params.set_output("result", Eigen::VectorXd(a + b));
        return true;
    });

    descriptor->register_node(add_node);

    auto tree = create_node_tree(descriptor);

    NodeTreeExecutorDesc desc;
    desc.policy = NodeTreeExecutorDesc::Policy::Eager;
    auto executor = create_node_tree_executor(desc);

    auto node = tree->add_node("add");

    executor->prepare_tree(tree.get());

    auto a = node->get_input_socket("a");
    auto b = node->get_input_socket("b");
    executor->sync_node_from_external_storage(
        a, Eigen::VectorXd(Eigen::Vector3d(1, 2, 3)));
    executor->sync_node_from_external_storage(
        b, Eigen::VectorXd(Eigen::Vector3d(1, 2, 3)));

    executor->execute_tree(tree.get());

    entt::meta_any result;
    executor->sync_node_to_external_storage(
        node->get_output_socket("result"), result);

    auto res = result.cast<Eigen::VectorXd>();

    std::cout << res;
}
