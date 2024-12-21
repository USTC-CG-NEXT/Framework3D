#include <Eigen/Eigen>
#include <autodiff/reverse/var.hpp>
#include <autodiff/reverse/var/eigen.hpp>
#include <iostream>

#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

using namespace autodiff;

NODE_DECLARATION_FUNCTION(L_BFGS)
{
    b.add_input<std::function<float(Eigen::VectorXf)>>("Cost function");
    b.add_input<Eigen::VectorXf>("Initial point");
    b.add_input<int>("Max iterations");
    b.add_input<float>("Tolerance");
    b.add_input<int>("Memory step size");
    b.add_output<Eigen::VectorXf>("Minimum point");
}

NODE_EXECUTION_FUNCTION(L_BFGS)
{
    auto f0 = params.get_input<std::function<float(Eigen::VectorXf)>>(
        "Cost function");

    auto f = [f0](const ArrayXvar& x) -> var {
        Eigen::VectorXf x_old = x.template cast<float>();
        float y = f0(x_old);
        return var(y);
    };
    auto x0 = params.get_input<Eigen::VectorXf>("Initial point");

    int max_iterations = params.get_input<int>("Max iterations");
    float tolerance = params.get_input<float>("Tolerance");
    int m = params.get_input<int>("Memory step size");

    VectorXvar x = x0.template cast<var>();

    Eigen::VectorXf x_old = x0;
    Eigen::VectorXf x_new = x0;

    var u;
    Eigen::VectorXf g;
    Eigen::VectorXf p;
    Eigen::VectorXf r;

    int n = x0.size();
    Eigen::MatrixXf g_set(max_iterations + 1, n);
    Eigen::MatrixXf x_set(max_iterations + 1, n);
    Eigen::MatrixXf s(max_iterations, n);
    Eigen::MatrixXf y(max_iterations, n);
    Eigen::VectorXf rho(max_iterations);
    Eigen::VectorXf alpha(max_iterations);

    x_set.row(0) = x0.transpose();

    for (int i = 0; i <= max_iterations; ++i) {
        if (i == 0) {
            u = f(x);
            Eigen::VectorXd g0 = gradient(u, x);
            g = g0.cast<float>();
            p = -g;
            x_new = x_old + p;
            x = x_new.template cast<var>();

            g_set.row(0) = g.transpose();
            x_set.row(1) = x_new.transpose();
        }
        else {
            u = f(x);
            Eigen::VectorXd g0 = gradient(u, x);
            g = g0.cast<float>();
            g_set.row(i) = g.transpose();

            s.row(i - 1) = x_set.row(i) - x_set.row(i - 1);
            y.row(i - 1) = g_set.row(i) - g_set.row(i - 1);

            rho[i - 1] = 1.0 / (y.row(i - 1).dot(s.row(i - 1)));

            Eigen::VectorXf q = g;
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

            float c1 = 1e-3;
            float alpha_ls = 1.0;
            float fx_old = val(f(x));
            for (int ls_iter = 0; ls_iter < 100; ++ls_iter) {
                x_new = x_old + alpha_ls * p;
                x = x_new.template cast<var>();
                float fx_new = val(f(x));
                if (fx_new <= fx_old + c1 * alpha_ls * g.dot(p)) {
                    break;
                }
                alpha_ls *= 0.5;
            }

            x_new = x_old + alpha_ls * p;
            x = x_new.template cast<var>();

            Eigen::VectorXf d = x_new - x_old;
            if (d.norm() < tolerance) {
                break;
            }
            x_set.row(i) = x_new.transpose();
        }
    }

    params.set_output<Eigen::VectorXf>("Minimum point", std::move(x_new));
    return true;
}

NODE_DECLARATION_UI(L_BFGS);
NODE_DEF_CLOSE_SCOPE
