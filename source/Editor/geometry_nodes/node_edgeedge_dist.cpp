#include "nodes/core/def/node_def.hpp"
#include <Eigen/Dense>
#include <Eigen/Sparse>

using Vec=Eigen::Vector3d;
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(edgeedge_dist)
{
    b.add_input<Vec>("Edge1_p1");
    b.add_input<Vec>("Edge1_p2");
    b.add_input<Vec>("Edge2_p3");
    b.add_input<Vec>("Edge2_p4");
    b.add_output<double>("distance");
}
NODE_EXECUTION_FUNCTION(edgeedge_dist)
{
    auto p1 = params.get_input<Vec>("Edge1_p1");
    auto p2 = params.get_input<Vec>("Edge1_p2");
    auto p3 = params.get_input<Vec>("Edge2_p3");
    auto p4 = params.get_input<Vec>("Edge2_p4");
    Vec d1 = p2 - p1;
    Vec d2 = p3 - p4;//direction vector
    Vec p1p3 = p3 - p1;
    double distance = 0;
    if ((d1.cross(d2)).norm() == 0) {//Two edges are parallel
        distance = (p1p3.cross(d1)).norm() / d1.norm();
    }
    else {
        distance = std::abs(p1p3.dot(d1.cross(d2))) / (d1.cross(d2)).norm();
    }

    params.set_output<double>("distance", std::move(distance));
    return true;
}
NODE_DECLARATION_UI(edgeedge_dist);
NODE_DEF_CLOSE_SCOPE