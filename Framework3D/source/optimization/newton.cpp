#include <Eigen/Eigen>
#include <autodiff/reverse/var.hpp>
#include <autodiff/reverse/var/eigen.hpp>

#include "nodes/core/def/node_def.hpp"

using namespace autodiff;

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(newton)
{
    b.add_input<std::function<float(Eigen::VectorXf)>>("Cost function");
    b.add_input<Eigen::VectorXf>("Initial point");
    b.add_input<int>("Max iterations");
    b.add_input<float>("Tolerance");
    b.add_output<Eigen::VectorXf>("Minimum point");
}

NODE_EXECUTION_FUNCTION(newton)
{
    auto f0 = params.get_input<std::function<float(Eigen::VectorXf)>>(
        "Cost function");

    auto f = [f0](const ArrayXvar& x) -> var {
        Eigen::VectorXf x_old = x.template cast<float>();
        float y = f0(x_old);
        return var(y);
    };

    auto x0 = params.get_input<Eigen::VectorXf>("Initial point");

    VectorXvar x = x0.template cast<var>();

    const int max_iterations = params.get_input<int>("Max iterations");
    const float tolerance = params.get_input<float>("Tolerance");

    Eigen::VectorXf x_old = x0;
    Eigen::VectorXf x_new = x0;
    for (int i = 0; i < max_iterations; ++i) {
        var u = f(x);
        Eigen::VectorXd g;
        Eigen::MatrixXd H = hessian(u, x, g);
        Eigen::VectorXd d0 = H.colPivHouseholderQr().solve(g);
        Eigen::VectorXf d = d0.cast<float>();
        x_new = x_old - d;
        if (d.norm() < tolerance) {
            break;
        }
        x_old = x_new;
        x = x_new.template cast<var>();
    }
    params.set_output<Eigen::VectorXf>("Minimum point", std::move(x_new));
    return true;
}

NODE_DECLARATION_UI(newton);
NODE_DEF_CLOSE_SCOPE