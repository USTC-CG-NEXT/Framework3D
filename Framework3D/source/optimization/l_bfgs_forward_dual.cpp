#include <Eigen/Eigen>
#include <autodiff/forward/dual.hpp>
#include <autodiff/forward/dual/eigen.hpp>
#include <iostream>

#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

using namespace autodiff;

NODE_DECLARATION_FUNCTION(l_bfgs_forward_dual)
{
    b.add_input<std::function<dual(const ArrayXdual&)>>("Cost function");
    // b.add_input<Eigen::VectorXd>("Initial point");
    // b.add_input<int>("Max iterations");
    // b.add_input<double>("Tolerance");
    // b.add_input<int>("Memory step size");
    b.add_output<Eigen::VectorXd>("Minimum point");
    b.add_output<double>("Minimum");
}

NODE_EXECUTION_FUNCTION(l_bfgs_forward_dual)
{
    auto f = params.get_input<std::function<dual(const ArrayXdual&)>>(
        "Cost function");

    // Eigen::VectorXd x0 = params.get_input<Eigen::VectorXd>("Initial point");
    // int max_iterations = params.get_input<int>("Max iterations");
    // double tolerance = params.get_input<double>("Tolerance");
    // int m = params.get_input<int>("Memory step size");
    Eigen::VectorXd x0(3);
    x0 << 1, 1, 1;
    int max_iterations = 100;
    double tolerance = 1e-6;
    int m = 5;

    VectorXdual x = x0.template cast<dual>();

    Eigen::VectorXd x_old = x0;
    Eigen::VectorXd x_new = x0;

    dual u;
    Eigen::VectorXd g;
    Eigen::VectorXd p;
    Eigen::VectorXd r;

    int n = x0.size();
    Eigen::MatrixXd g_set(max_iterations + 1, n);
    Eigen::MatrixXd x_set(max_iterations + 1, n);
    Eigen::MatrixXd s(max_iterations, n);
    Eigen::MatrixXd y(max_iterations, n);
    Eigen::VectorXd rho(max_iterations);
    Eigen::VectorXd alpha(max_iterations);

    x_set.row(0) = x0.transpose();

    for (int i = 0; i <= max_iterations; ++i) {
        if (i == 0) {
            gradient(f, wrt(x), at(x), u, g);
            p = -g;
            x_new = x_old + p;
            x = x_new.template cast<dual>();

            g_set.row(0) = g.transpose();
            x_set.row(1) = x_new.transpose();
        }
        else {
            gradient(f, wrt(x), at(x), u, g);
            g_set.row(i) = g.transpose();

            s.row(i - 1) = x_set.row(i) - x_set.row(i - 1);
            y.row(i - 1) = g_set.row(i) - g_set.row(i - 1);

            rho[i - 1] = 1.0 / (y.row(i - 1).dot(s.row(i - 1)));

            Eigen::VectorXd q = g;
            int bound = std::min(i, m);

            for (int j = i - 1; j >= std::max(0, i - bound); j--) {
                alpha[j] = rho[j] * s.row(j).dot(q);
                q -= alpha[j] * y.row(j).transpose();
            }

            r = q;
            for (int j = std::max(0, i - bound); j < i; j++) {
                double beta = rho[j] * y.row(j).dot(r);
                r += s.row(j).transpose() * (alpha[j] - beta);
            }

            p = -r;

            x_old = x_new;

            double c1 = 1e-3;
            double alpha_ls = 1.0;
            double fx_old = val(f(x));
            for (int ls_iter = 0; ls_iter < 100; ++ls_iter) {
                x_new = x_old + alpha_ls * p;
                x = x_new.template cast<dual>();
                float fx_new = val(f(x));
                if (fx_new <= fx_old + c1 * alpha_ls * g.dot(p)) {
                    break;
                }
                alpha_ls *= 0.5;
            }

            x_new = x_old + alpha_ls * p;
            x = x_new.template cast<dual>();

            Eigen::VectorXd d = x_new - x_old;
            if (d.norm() < tolerance) {
                break;
            }
            x_set.row(i) = x_new.transpose();
        }
    }
    u = f(x);
    double result = val(u);

    params.set_output<Eigen::VectorXd>("Minimum point", std::move(x_new));
    params.set_output<double>("Minimum", std::move(result));

    return true;
}
NODE_DECLARATION_REQUIRED(l_bfgs_forward_dual);
NODE_DECLARATION_UI(l_bfgs_forward_dual);
NODE_DEF_CLOSE_SCOPE
