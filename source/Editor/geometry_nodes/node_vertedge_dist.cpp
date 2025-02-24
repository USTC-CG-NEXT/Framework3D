#include "nodes/core/def/node_def.hpp"
#include <Eigen/Dense>
#include <Eigen/Sparse>

using Vec = Eigen::Vector3d;
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(vertedge_dist)
{
    b.add_input<Vec>("vertex");
    b.add_input<Vec>("Edge_v1");
    b.add_input<Vec>("Edge_v2");
    b.add_output<double>("distance");
}
NODE_EXECUTION_FUNCTION(vertedge_dist)
{
    auto vertex = params.get_input<Vec>("vertex");
    auto v1 = params.get_input<Vec>("Edge_v1");
    auto v2 = params.get_input<Vec>("Edge_v2");
    double distance = 0;
    Vec d1 = v2 - v1; //direction vector one
    Vec d2 = vertex - v1;//direction vector two
    distance = (d1.cross(d2)).norm() / d1.norm();

    params.set_output<double>("distance", std::move(distance));
    return true;
}
NODE_DECLARATION_UI(vertedge_dist);
NODE_DEF_CLOSE_SCOPE