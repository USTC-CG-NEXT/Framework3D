#include <Eigen/Eigen>
#include <autodiff/forward/dual.hpp>
#include <autodiff/forward/dual/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_function_6)
{
    b.add_output<std::function<dual(dual)>>("Function");
}

NODE_EXECUTION_FUNCTION(test_function_6)
{
    auto f = [](dual x) { return 2 * x; };
    params.set_output<std::function<dual(dual)>>("Function", std::move(f));
    return true;
}

NODE_DECLARATION_UI(test_function_6);
NODE_DEF_CLOSE_SCOPE