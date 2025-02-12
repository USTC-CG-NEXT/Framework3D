#include <Eigen/Eigen>
#include <autodiff/forward/real.hpp>
#include <autodiff/forward/real/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_function_9)
{
    b.add_output<std::function<real(const ArrayXreal&)>>("Function");
}

NODE_EXECUTION_FUNCTION(test_function_9)
{
    auto f = [](const ArrayXreal& x) { return sqrt((x * x).sum()); };
    params.set_output<std::function<real(const ArrayXreal&)>>(
        "Function", std::move(f));
    return true;
}

NODE_DECLARATION_UI(test_function_9);
NODE_DEF_CLOSE_SCOPE