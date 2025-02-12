#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "nodes/core/def/node_def.hpp"


using Vec=Eigen::Vector3d;
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(vertvert_dist)
{
    b.add_input<Vec>("vertex1");
    b.add_input<Vec>("vertex2");
    b.add_output<double>("distance");
}
NODE_EXECUTION_FUNCTION(vertvert_dist)
{
    auto v1 = params.get_input<Vec>("vertex1");
    auto v2 = params.get_input<Vec>("vertex2");
    double distance = (v1-v2).norm();
    params.set_output<double>("distance", std::move(distance));
    return true;
}
NODE_DECLARATION_UI(vertvert_dist);
NODE_DEF_CLOSE_SCOPE