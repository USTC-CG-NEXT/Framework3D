
#include "GCore/GOP.h"
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(node_merge_geometry)
{
    // Function content omitted

    b.add_input_group<Geometry>("Geometries").set_runtime_dynamic(true);

    b.add_output<Geometry>("Geometry");
}

NODE_EXECUTION_FUNCTION(node_merge_geometry)
{
    // Function content omitted
    auto geometries = params.get_input_group<Geometry>("Geometries");
    return true;
}

NODE_DECLARATION_UI(node_merge_geometry);
NODE_DEF_CLOSE_SCOPE
