#include <iostream>
#include <string>

#include "nodes/core/def/node_def.hpp"
#include "polyscope/polyscope.h"
#include "polyscope_widget/polyscope_renderer.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(get_control_points)
{
    b.add_input<std::string>("Strcture Name");
    b.add_output<std::vector<size_t>>("Control Points Indices");
}

NODE_EXECUTION_FUNCTION(get_control_points)
{
    auto structure_name = params.get_input<std::string>("Strcture Name");
    structure_name = std::string(structure_name.c_str());

    bool is_empty = structure_name.empty();
    bool is_surface_mesh =
        polyscope::hasStructure("Surface Mesh", structure_name);
    bool is_curve_network =
        polyscope::hasStructure("Curve Network", structure_name);
    bool is_point_cloud =
        polyscope::hasStructure("Point Cloud", structure_name);

    if (is_empty ||
        (!is_surface_mesh && !is_curve_network && !is_point_cloud)) {
        std::cerr << "The structure is not found." << std::endl;
        return false;
    }

    std::vector<size_t> control_points =
        PolyscopeRenderer::GetControlPoints(structure_name);

    params.set_output("Control Points Indices", control_points);

    return true;
}

NODE_DECLARATION_UI(get_control_points);
NODE_DEF_CLOSE_SCOPE
