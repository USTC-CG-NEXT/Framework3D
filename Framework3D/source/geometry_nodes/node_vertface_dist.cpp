#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "nodes/core/def/node_def.hpp"
#include <cmath>
#include <limits>
#include <cstdlib>
using Vec = Eigen::Vector3d;
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(vertface_dist)
{
    
    b.add_input<Vec>("vertex");
    b.add_input<Vec>("Face_v0");
    b.add_input<Vec>("Face_v1");
    b.add_input<Vec>("Face_v2");
    b.add_output<double>("distance");
}
NODE_EXECUTION_FUNCTION(vertface_dist)
{
    auto vertex = params.get_input<Vec>("vertex");
    auto v0 = params.get_input<Vec>("Face_v0");
    auto v1 = params.get_input<Vec>("Face_v1");
    auto v2 = params.get_input<Vec>("Face_v2");//3 points of the triangle face
    double distance = 0;
    Vec vec1 = v1 - v0;
    Vec vec2 = v2 - v0;
    Vec normal = vec1.cross(vec2);
    distance = std::abs(normal.dot(v0 - vertex) / normal.norm());

    
    params.set_output<double>("distance", std::move(distance));
    return true;
}

NODE_DECLARATION_UI(vertface_dist);
NODE_DEF_CLOSE_SCOPE