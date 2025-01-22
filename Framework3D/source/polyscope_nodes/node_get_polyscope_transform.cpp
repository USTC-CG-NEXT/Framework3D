#include <string>

#include "nodes/core/def/node_def.hpp"
#include "polyscope/polyscope.h"


NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(get_polyscope_transform)
{
    b.add_input<std::string>("Structure Name");

    b.add_output<glm::mat4x4>("Transform");
}

NODE_EXECUTION_FUNCTION(get_polyscope_transform)
{
    auto name = params.get_input<std::string>("Structure Name");
    // name.size()为255，需要修正
    name = std::string(name.c_str());

    polyscope::Structure* structure = nullptr;

    if (polyscope::hasStructure("Surface Mesh", name)) {
        structure = polyscope::getStructure("Surface Mesh", name);
    }
    else if (polyscope::hasStructure("Point Cloud", name)) {
        structure = polyscope::getStructure("Point Cloud", name);
    }
    else if (polyscope::hasStructure("Curve Network", name)) {
        structure = polyscope::getStructure("Curve Network", name);
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