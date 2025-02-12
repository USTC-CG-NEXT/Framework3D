#include <Eigen/Eigen>
#include <autodiff/forward/dual.hpp>
#include <autodiff/forward/dual/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_function_11)
{
    b.add_input<std::function<dual(const ArrayXdual&)>>("Function_in");
    b.add_output<std::function<dual(const ArrayXdual&)>>("Function_result");
}

NODE_EXECUTION_FUNCTION(test_function_11)
{
    auto f =
        params.get_input<std::function<dual(const ArrayXdual&)>>("Function_in");
    auto g = [f](const ArrayXdual& x) {
        dual y = 2 * f(x);
        return y;
    };
    params.set_output<std::function<dual(const ArrayXdual&)>>(
        "Function_result", std::move(g));
    return true;
}

NODE_DECLARATION_UI(test_function_11);
NODE_DEF_CLOSE_SCOPE