#include <Eigen/Eigen>
#include <autodiff/forward/dual.hpp>
#include <autodiff/forward/dual/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(gradient_forward_dual)
{
    b.add_input<std::function<dual(const ArrayXdual&)>>("Function");
    //    b.add_input<Eigen::VectorXd>("Target Point");
    b.add_output<Eigen::VectorXd>("Gradient");
}

NODE_EXECUTION_FUNCTION(gradient_forward_dual)
{
    auto f =
        params.get_input<std::function<dual(const ArrayXdual&)>>(
        "Function");
    Eigen::VectorXd x0(3);
    //    Eigen::VectorXd x0 = params.get_input<Eigen::VectorXd>("Target
    //    Point");
    x0 << 1, 2, 3;
    ArrayXdual x = x0.template cast<dual>();
    dual y;
    Eigen::VectorXd g;
    gradient(f, wrt(x), at(x), y, g);

    params.set_output<Eigen::VectorXd>("Gradient", std::move(g));

    return true;
}

NODE_DECLARATION_REQUIRED(gradient_forward_dual);
NODE_DECLARATION_UI(gradient_forward_dual);
NODE_DEF_CLOSE_SCOPE