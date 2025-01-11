#include "GCore/GOP.h"
#include "GCore/geom_payload.hpp"
#include "nodes/core/def/node_def.hpp"
#include "polyscope/polyscope.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(get_polyscope_transform)
{
    b.add_output<glm::mat4x4>("Transform");
}

NODE_EXECUTION_FUNCTION(get_polyscope_transform)
{
    auto global_payload = params.get_global_payload<GeomPayload>();
    auto stage = global_payload.stage;
    auto sdf_path = global_payload.prim_path;

    polyscope::Structure* structure = nullptr;

    if (polyscope::hasStructure("Surface Mesh", sdf_path.GetName())) {
        structure = polyscope::getStructure("Surface Mesh", sdf_path.GetName());
    }
    else if (polyscope::hasStructure("Point Cloud", sdf_path.GetName())) {
        structure = polyscope::getStructure("Point Cloud", sdf_path.GetName());
    }
    else if (polyscope::hasStructure("Curve Network", sdf_path.GetName())) {
        structure =
            polyscope::getStructure("Curve Network", sdf_path.GetName());
    }

    if (!structure) {
        return false;
    }

    auto transform = structure->getTransform();

    params.set_output("Transform", transform);

    return true;
}

NODE_DECLARATION_UI(get_polyscope_transform);
NODE_DEF_CLOSE_SCOPE