#include <Eigen/Eigen>
#include "nodes/core/def/node_def.hpp"
#include <autodiff/reverse/var.hpp>
#include <autodiff/reverse/var/eigen.hpp>

NODE_DEF_OPEN_SCOPE

using namespace autodiff;

NODE_DECLARATION_FUNCTION(L_BFGS)
{
    b.add_input<std::function<double(Eigen::Vector3d)>>("Cost function");
    b.add_input<Eigen::Vector3d>("Initial point");
    b.add_input<int>("Max iterations");
    b.add_input<double>("Tolerance");
    b.add_input<int>("Memory step size");
    b.add_output<Eigen::Vector3d>("Minimum point");
}

NODE_EXECUTION_FUNCTION(L_BFGS)
{
    auto f0 = params.get_input<std::function<double(Eigen::Vector3d)>>(
        "Cost function");

    auto f = [f0](const ArrayXvar& x) -> var {
        Eigen::Vector3d x_old;
        x_old[0] = val(x[0]);
        x_old[1] = val(x[1]);
        x_old[2] = val(x[2]);
        double y = f0(x_old);
        return var(y);
    };
    auto x0 = params.get_input<Eigen::Vector3d>("Initial point");

    int max_iterations = params.get_input<int>("Max iterations");
    double tolerance = params.get_input<double>("Tolerance");
    int m = params.get_input<int>("Memory step size");

    VectorXvar x(3);

    Eigen::Vector3d x_old = x0;
    Eigen::Vector3d x_new;

    var u;
    Eigen::Vector3d g;
    Eigen::Vector3d p;
    Eigen::Vector3d r;

    Eigen::MatrixXd g_set(max_iterations + 1, 3);
    Eigen::MatrixXd x_set(max_iterations + 1, 3);
    Eigen::MatrixXd s(max_iterations + 1, 3);
    Eigen::MatrixXd y(max_iterations + 1, 3);
    Eigen::VectorXd rho(max_iterations + 1);
    Eigen::VectorXd alpha(max_iterations + 1);

    x_set(0, 0) = x0[0];
    x_set(0, 1) = x0[1];
    x_set(0, 2) = x0[2];

    x << x0[0], x0[1], x0[2];

    for (int i = 0; i <= max_iterations; ++i) {   
        if (i == 0) {
            u = f(x);
            g = gradient(u, x);
            p = -g;
            x_new = x_old + p;
            x << x_new[0], x_new[1], x_new[2];

            g_set(0, 0) = g[0];
            g_set(0, 1) = g[1];
            g_set(0, 2) = g[2];
            x_set(1, 0) = x_new[0];
            x_set(1, 1) = x_new[1];
            x_set(1, 2) = x_new[2];
        }
        else {
            u = f(x);
            g = gradient(u, x);
            g_set(i, 0) = g[0];
            g_set(i, 1) = g[1];
            g_set(i, 2) = g[2];

            s(i - 1, 0) = x_set(i, 0) - x_set(i - 1, 0);
            s(i - 1, 1) = x_set(i, 1) - x_set(i - 1, 1);
            s(i - 1, 2) = x_set(i, 2) - x_set(i - 1, 2);

            y(i - 1, 0) = g_set(i, 0) - g_set(i - 1, 0);
            y(i - 1, 1) = g_set(i, 1) - g_set(i - 1, 1);
            y(i - 1, 2) = g_set(i, 2) - g_set(i - 1, 2);

            rho[i - 1] =
                1 / (y(i - 1, 0) * s(i - 1, 0) + y(i - 1, 1) * s(i - 1, 1) +
                     y(i - 1, 2) * s(i - 1, 2));

            Eigen::Vector3d q = g;
            if (i < m) {
                for (int j = i - 1; j >= 0; j++) {
                    alpha[j] = rho(j) * (s(j, 0) * q[0] + s(j, 1) * q[1] +
                                         s(j, 2) * q[2]);
                    q[0] = q[0] - alpha[j] * y(j, 0);
                    q[1] = q[1] - alpha[j] * y(j, 1);
                    q[2] = q[2] - alpha[j] * y(j, 2);
                }
                r = q;
                for (int j = 0; j <= i - 1; j++) {
                    double beta = rho[j] * (y(j, 0) * r[0] + y(j, 1) * r[1] +
                                            y(j, 2) * r[2]);
                    r[0] = r[0] + s(j, 0) * (alpha[j] - beta);
                    r[1] = r[1] + s(j, 1) * (alpha[j] - beta);
                    r[2] = r[2] + s(j, 2) * (alpha[j] - beta);
                }
            }
            else {
                for (int j = i - 1; j >= i - m; j++) {
                    alpha[j] = rho(j) * (s(j, 0) * q[0] + s(j, 1) * q[1] +
                                         s(j, 2) * q[2]);
                    q[0] = q[0] - alpha[j] * y(j, 0);
                    q[1] = q[1] - alpha[j] * y(j, 1);
                    q[2] = q[2] - alpha[j] * y(j, 2);
                }
                r = q;
                for (int j = i - m; j <= i - 1; j++) {
                    double beta = rho[j] * (y(j, 0) * r[0] + y(j, 1) * r[1] +
                                            y(j, 2) * r[2]);
                    r[0] = r[0] + s(j, 0) * (alpha[j] - beta);
                    r[1] = r[1] + s(j, 1) * (alpha[j] - beta);
                    r[2] = r[2] + s(j, 2) * (alpha[j] - beta);
                }
            }
            p = -r;
            
            x_old = x_new;

            //Armijo
            double c1 = 1e-4;
            double alpha_ls = 1.0;
            double fx_old = val(f(x));
            for (int ls_iter = 0; ls_iter < 100; ++ls_iter) {
                x_new = x_old + alpha_ls * p;
                x << x_new[0], x_new[1], x_new[2];
                double fx_new = val(f(x));
                if (fx_new <= fx_old + c1 * alpha_ls * g.dot(p)) {
                    break;
                }
                alpha_ls *= 0.5;
            }

            x_new = x_old + alpha_ls * p;
            x << x_new[0], x_new[1], x_new[2];

            Eigen::Vector3d d = x_new - x_old;
            if (d.norm() < tolerance) {
                break;
            }
            x_set(i, 0) = x_new[0];
            x_set(i, 1) = x_new[1];
            x_set(i, 2) = x_new[2];
        }     
    }
    params.set_output<Eigen::Vector3d>("Minimum point", std::move(x_new));
}


NODE_DECLARATION_UI(L_BFGS);
NODE_DEF_CLOSE_SCOPE