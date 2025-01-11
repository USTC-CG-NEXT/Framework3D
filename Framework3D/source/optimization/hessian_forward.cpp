#include <Eigen/Eigen>
#include <autodiff/forward/real.hpp>
#include <autodiff/forward/real/eigen.hpp>

#include "nodes/core/def/node_def.hpp"
using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(hessian_forward)
{
    b.add_input<std::function<real(const ArrayXreal&)>>("Function");
    // b.add_input<Eigen::VectorXd>("Target Point");
    b.add_output<Eigen::MatrixXd>("Gradient");
}

NODE_EXECUTION_FUNCTION(hessian_forward)
{
    auto f = params.get_input<std::function<real(const ArrayXreal&)>>(
        "Function");
    Eigen::VectorXd x0(3);
    x0 << 1, 2, 3;
    // Eigen::VectorXd x0 = params.get_input<Eigen::VectorXd>("Target Point");
    ArrayXreal x = x0.template cast<real>();
    real y;
    Eigen::VectorXd g;
    Eigen::MatrixXd H;
    //hessian(f, wrt(x), at(x), y, g, H);

    params.set_output<Eigen::MatrixXd>("Hessian", std::move(H));

    return true;
}

NODE_DECLARATION_REQUIRED(hessian_forward);
NODE_DECLARATION_UI(hessian_forward);
NODE_DEF_CLOSE_SCOPE