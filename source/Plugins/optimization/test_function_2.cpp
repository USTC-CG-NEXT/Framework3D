#include <Eigen/Eigen>
#include <autodiff/reverse/var.hpp>
#include <autodiff/reverse/var/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_function_2)
{
    b.add_output<std::function<var(const ArrayXvar&)>>("Function");
}

NODE_EXECUTION_FUNCTION(test_function_2)
{
    auto f = [](const ArrayXvar& x) {
        return sqrt((x * x).sum());
    };
    params.set_output<std::function<var(const ArrayXvar&)>>(
        "Function", std::move(f));
    return true;
}

NODE_DECLARATION_UI(test_function_2);
NODE_DEF_CLOSE_SCOPE