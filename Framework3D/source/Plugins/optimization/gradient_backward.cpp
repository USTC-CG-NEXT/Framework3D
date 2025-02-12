#include <Eigen/Eigen>
#include <autodiff/reverse/var.hpp>
#include <autodiff/reverse/var/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(gradient_backward)
{
    b.add_input<std::function<var(const ArrayXvar&)>>("Function");
//    b.add_input<Eigen::VectorXd>("Target Point");
    b.add_output<Eigen::VectorXd>("Gradient");
}

NODE_EXECUTION_FUNCTION(gradient_backward)
{
    auto f = params.get_input<std::function<var(const ArrayXvar&)>>("Function");
    Eigen::VectorXd x0(3);
//    Eigen::VectorXd x0 = params.get_input<Eigen::VectorXd>("Target Point");
    x0 << 1, 2, 3;
    ArrayXvar x = x0.template cast<var>();
    var y = f(x);
    Eigen::VectorXd g = gradient(y, x);

    params.set_output<Eigen::VectorXd>("Gradient", std::move(g));

    return true;
}

NODE_DECLARATION_REQUIRED(gradient_backward);
NODE_DECLARATION_UI(gradient_backward);
NODE_DEF_CLOSE_SCOPE