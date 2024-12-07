#include <Eigen/Eigen>
#include "nodes/core/def/node_def.hpp"
#include <autodiff/reverse/var.hpp>
#include <autodiff/reverse/var/eigen.hpp>

NODE_DEF_OPEN_SCOPE

using namespace autodiff;

NODE_DECLARATION_FUNCTION(newton)
{
    b.add_input<std::function<double(Eigen::Vector3d)>>("Cost function");
    b.add_input<Eigen::Vector3d>("Initial point");
    b.add_output<Eigen::Vector3d>("Minimum point");
}

NODE_EXECUTION_FUNCTION(newton)
{
    auto f =
        params.get_input<std::function<var(const ArrayXvar&)>>("Cost function");
    auto x0 = params.get_input<Eigen::Vector3d>("Initial point");

    VectorXvar x(3);
    const int max_iterations = 50;
    const double tolerance = 1e-6;

    Eigen::Vector3d x_old = x0;
    Eigen::Vector3d x_new;

    x << x0[0], x0[1], x0[2];

    for (int i = 0; i < max_iterations; ++i) {
        var u = f(x);

        Eigen::Vector3d g;
        Eigen::Matrix3d H = hessian(u, x, g);
        Eigen::Vector3d d = H.colPivHouseholderQr().solve(g);

        x_new = x_old - d;
        if (d.norm() < tolerance) {
            break;
        }
        x_old = x_new;
        x << x_new[0], x_new[1], x_new[2];
    }
    params.set_output<Eigen::Vector3d>("Minimum point", std::move(x_new));

}

NODE_DECLARATION_UI(newton);
NODE_DEF_CLOSE_SCOPE