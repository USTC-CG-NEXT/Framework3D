#include <Eigen/Eigen>
#include <autodiff/reverse/var.hpp>
#include <autodiff/reverse/var/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(function_add)
{
    b.add_input<std::function<var(const ArrayXvar&)>>("Function_1");
    b.add_input<std::function<var(const ArrayXvar&)>>("Function_2");
    b.add_output<std::function<var(const ArrayXvar&)>>("Function_result");
}

NODE_EXECUTION_FUNCTION(function_add)
{
    auto f1 =
        params.get_input<std::function<var(const ArrayXvar&)>>("Function_1");
    auto f2 =
        params.get_input<std::function<var(const ArrayXvar&)>>("Function_2");
    auto f = [f1, f2](const ArrayXvar& x) {
        var y = f1(x) + f2(x);
        return y;
    };
    params.set_output<std::function<var(const ArrayXvar&)>>(
        "Function_result", std::move(f));
    return true;
}

NODE_DECLARATION_UI(function_add);
NODE_DEF_CLOSE_SCOPE