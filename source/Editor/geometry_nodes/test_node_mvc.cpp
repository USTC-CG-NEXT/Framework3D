
#include "nodes/core/def/node_def.hpp"
#include <Eigen/Eigen>
#include <functional>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(test_node_mvc)
{
    b.add_input<Eigen::MatrixXd>("Vertices");
    b.add_input<std::function<Eigen::MatrixXd(const Eigen::MatrixXd&)>>("GBC");
    b.add_output<Eigen::MatrixXd>("Outcomes");
}

NODE_EXECUTION_FUNCTION(test_node_mvc)
{
    auto V = params.get_input<Eigen::MatrixXd>("Vertices");
    auto GBC = params.get_input <
               std::function<Eigen::MatrixXd(const Eigen::MatrixXd&)>>("GBC");
    Eigen::MatrixXd Outcomes = GBC(V);
    params.set_output("Outcomes", Outcomes);
    return true;
}

NODE_DECLARATION_UI(test_node_mvc);
NODE_DEF_CLOSE_SCOPE
