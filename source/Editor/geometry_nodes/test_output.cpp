
#include "nodes/core/def/node_def.hpp"
#include <Eigen/Eigen>
#include <Eigen/Dense>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(test_output)
{
    b.add_input<Eigen::MatrixXd>("Input");
}

NODE_EXECUTION_FUNCTION(test_output)
{
    auto Output = params.get_input<Eigen::MatrixXd>("Input");
    std::cout << Output << std::endl;
    return true;
}

NODE_DECLARATION_UI(test_output);
NODE_DEF_CLOSE_SCOPE
