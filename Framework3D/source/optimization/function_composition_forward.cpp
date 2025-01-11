#include <Eigen/Eigen>
#include <autodiff/forward/real.hpp>
#include <autodiff/forward/real/eigen.hpp>

#include "nodes/core/def/node_def.hpp"

using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(function_composition_forward)
{
    b.add_input<std::function<real(const ArrayXreal&)>>("Function_1");
    b.add_input<std::function<real(real)>>("Function_2");
    b.add_output<std::function<real(const ArrayXreal&)>>("Function_result");
}

NODE_EXECUTION_FUNCTION(function_composition_forward)
{
    auto f1 =
        params.get_input<std::function<real(const ArrayXreal&)>>("Function_1");
    auto f2 = params.get_input<std::function<real(real)>>("Function_2");
    auto f = [f1, f2](const ArrayXreal& x) {
        real y = f2(f1(x));
        return y;
    };
    params.set_output<std::function<real(const ArrayXreal&)>>(
        "Function_result", std::move(f));
    return true;
}

NODE_DECLARATION_UI(function_composition_forward);
NODE_DEF_CLOSE_SCOPE