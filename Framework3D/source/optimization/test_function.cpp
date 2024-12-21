#include <Eigen/Eigen>
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_function)
{
    b.add_output<std::function<float(Eigen::VectorXf)>>("Function");
}

NODE_EXECUTION_FUNCTION(test_function)
{
    auto f = [](Eigen::VectorXf x) { return x[0] * x[0] + x[1] * x[1] + 1;
    };
    params.set_output<std::function<float(Eigen::VectorXf)>>(
        "Function", std::move(f));
    return true;
}

NODE_DECLARATION_UI(test_function);
NODE_DEF_CLOSE_SCOPE