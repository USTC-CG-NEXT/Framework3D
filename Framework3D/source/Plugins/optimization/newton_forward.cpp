#include <Eigen/Eigen>
#include <autodiff/forward/dual.hpp>
#include <autodiff/forward/dual/eigen.hpp>

#include "nodes/core/def/node_def.hpp"

using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(newton_forward)
{
    b.add_input<std::function<dual2nd(const ArrayXdual2nd&)>>("Cost function");
    // b.add_input<Eigen::VectorXd>("Initial point");
    // b.add_input<int>("Max iterations");
    // b.add_input<double>("Tolerance");
    b.add_output<Eigen::VectorXd>("Minimum point");
    b.add_output<double>("Minimum");
}

NODE_EXECUTION_FUNCTION(newton_forward)
{
    auto f = params.get_input<std::function<dual2nd(const ArrayXdual2nd&)>>(
        "Cost function");
    // Eigen::VectorXd x0 = params.get_input<Eigen::VectorXd>("Initial point");
    // const int max_iterations = params.get_input<int>("Max iterations");
    // const double tolerance = params.get_input<double>("Tolerance");
    Eigen::VectorXd x0(3);
    x0 << 1, 1, 1;
    const int max_iterations = 100;
    const double tolerance = 1e-6;

    VectorXdual2nd x = x0.template cast<dual2nd>();

    Eigen::VectorXd x_old = x0;
    Eigen::VectorXd x_new = x0;
    for (int i = 0; i < max_iterations; ++i) {
        dual2nd u = f(x);
        Eigen::VectorXd g;
        Eigen::MatrixXd H;
        hessian(f, wrt(x), at(x), u, g, H);
        Eigen::VectorXd d = H.colPivHouseholderQr().solve(g);
        x_new = x_old - d;
        if (d.norm() < tolerance) {
            break;
        }
        x_old = x_new;
        x = x_new.template cast<dual2nd>();
    }
    dual2nd u = f(x);
    double result = val(u);

    params.set_output<Eigen::VectorXd>("Minimum point", std::move(x_new));
    params.set_output<double>("Minimum", std::move(result));

    return true;
}

NODE_DECLARATION_REQUIRED(newton_forward);
NODE_DECLARATION_UI(newton_forward);
NODE_DEF_CLOSE_SCOPE