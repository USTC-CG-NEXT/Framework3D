#include <Eigen/Eigen>
#include <autodiff/reverse/var.hpp>
#include <autodiff/reverse/var/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(test_function_3)
{
    b.add_output<std::function<var(var)>>("Function");
}

NODE_EXECUTION_FUNCTION(test_function_3)
{
    auto f = [](var x) { return 2 * x; };
    params.set_output<std::function<var(var)>>("Function", std::move(f));
    return true;
}

NODE_DECLARATION_UI(test_function_3);
NODE_DEF_CLOSE_SCOPE