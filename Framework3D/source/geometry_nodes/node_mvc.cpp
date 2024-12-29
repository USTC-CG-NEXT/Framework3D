#include <igl/mvc.h>

#include <Eigen/Eigen>
#include <functional>

#include "nodes/core/def/node_def.hpp"

std::function<Eigen::MatrixXd(const Eigen::MatrixXd&)> generate_weight_function(
    const Eigen::MatrixXd& C)
{
    return [C](const Eigen::MatrixXd& V) -> Eigen::MatrixXd {
        Eigen::MatrixXd W;
        igl::mvc(V, C, W);
        return W;
    };
}

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(node_mvc)
{
    // Function content omitted
    b.add_input<Eigen::MatrixXd>("Control Points");
    b.add_output<std::function<Eigen::MatrixXd(const Eigen::MatrixXd&)>>("MVC");
}

NODE_EXECUTION_FUNCTION(node_mvc)
{
    // Function content omitted
    auto C = params.get_input<Eigen::MatrixXd>("Control Points");

    // at least three control points
    assert(C.rows() > 2);

    // dimension of points
    assert(C.cols() == 3 || C.cols() == 2);

    auto W = generate_weight_function(C);
    // we've made W transpose
    params.set_output("MVC", W);
    return true;
}

NODE_DECLARATION_UI(node_mvc);
NODE_DEF_CLOSE_SCOPE