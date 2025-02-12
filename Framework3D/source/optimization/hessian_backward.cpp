#include <Eigen/Eigen>
#include <autodiff/reverse/var.hpp>
#include <autodiff/reverse/var/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(hessian_backward)
{
    b.add_input<std::function<var(const ArrayXvar&)>>("Function");
    //b.add_input<Eigen::VectorXd>("Target Point");
    b.add_output<Eigen::MatrixXd>("Hessian");
}

NODE_EXECUTION_FUNCTION(hessian_backward)
{
    auto f = params.get_input<std::function<var(const ArrayXvar&)>>("Function");
    Eigen::VectorXd x0(3);
    x0 << 1, 2, 3;
   //Eigen::VectorXd x0 = params.get_input<Eigen::VectorXd>("Target Point");
    ArrayXvar x = x0.template cast<var>();
    var y = f(x);
    Eigen::VectorXd g;
    Eigen::MatrixXd H = hessian(y, x, g);

    params.set_output<Eigen::MatrixXd>("Hessian", std::move(H));

    return true;
}

NODE_DECLARATION_REQUIRED(hessian_backward);

NODE_DECLARATION_UI(hessian_backward);
NODE_DEF_CLOSE_SCOPE