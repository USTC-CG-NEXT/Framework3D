#include <Eigen/Eigen>
#include <autodiff/forward/dual.hpp>
#include <autodiff/forward/dual/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_function_5)
{
    b.add_output<std::function<dual(const ArrayXdual&)>>("Function");
}

NODE_EXECUTION_FUNCTION(test_function_5)
{
    auto f = [](const ArrayXdual& x) { return sqrt((x * x).sum()); };
    params.set_output<std::function<dual(const ArrayXdual&)>>(
        "Function", std::move(f));
    return true;
}

NODE_DECLARATION_UI(test_function_5);
NODE_DEF_CLOSE_SCOPE