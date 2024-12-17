#include <polyscope/point_cloud.h>

#include "nodes/core/def/node_def.hpp"
#include "polyscope_widget/polyscope_widget.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(create_point_cloud)
{
    b.add_input<float>("x min").default_val(-1);
    b.add_input<float>("x max").default_val(1);
    b.add_input<float>("y min").default_val(-1);
    b.add_input<float>("y max").default_val(1);
    b.add_input<float>("z min").default_val(-1);
    b.add_input<float>("z max").default_val(1);
    b.add_input<int>("Number of Points").default_val(2000);
    b.add_input<float>("Point Radius").default_val(0.02);
}

NODE_EXECUTION_FUNCTION(create_point_cloud)
{
    auto x_min = params.get_input<float>("x min");
    auto x_max = params.get_input<float>("x max");
    auto y_min = params.get_input<float>("y min");
    auto y_max = params.get_input<float>("y max");
    auto z_min = params.get_input<float>("z min");
    auto z_max = params.get_input<float>("z max");
    auto num_points = params.get_input<int>("Number of Points");
    auto point_radius = params.get_input<float>("Point Radius");

    std::vector<glm::vec3> points;

    // generate a point cloud
    for (int i = 0; i < num_points; i++) {
        points.push_back(glm::vec3{
            polyscope::randomReal(x_min, x_max),
            polyscope::randomReal(y_min, y_max),
            polyscope::randomReal(z_min, z_max),
        });
    }

    // Polyscope has been initialized in PolyscopeRenderer::PolyscopeRenderer
    auto point_cloud = polyscope::registerPointCloud("point cloud", points);

    point_cloud->setPointRadius(point_radius);
}

NODE_DECLARATION_REQUIRED(create_point_cloud);

NODE_DECLARATION_UI(create_point_cloud);

NODE_DEF_CLOSE_SCOPE