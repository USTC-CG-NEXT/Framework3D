#include <Eigen/Eigen>
#include <autodiff/forward/dual.hpp>
#include <autodiff/forward/dual/eigen.hpp>

#include "nodes/core/def/node_def.hpp"

using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(function_composition_forward_dual)
{
    b.add_input<std::function<dual(const ArrayXdual&)>>("Function_1");
    b.add_input<std::function<dual(dual)>>("Function_2");
    b.add_output<std::function<dual(const ArrayXdual&)>>("Function_result");
}

NODE_EXECUTION_FUNCTION(function_composition_forward_dual)
{
    auto f1 =
        params.get_input<std::function<dual(const ArrayXdual&)>>("Function_1");
    auto f2 = params.get_input<std::function<dual(dual)>>("Function_2");
    auto f = [f1, f2](const ArrayXdual& x) {
        dual y = f2(f1(x));
        return y;
    };
    params.set_output<std::function<dual(const ArrayXdual&)>>(
        "Function_result", std::move(f));
    return true;
}

NODE_DECLARATION_UI(function_composition_forward_dual);
NODE_DEF_CLOSE_SCOPE