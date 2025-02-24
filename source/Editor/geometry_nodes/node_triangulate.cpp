#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(triangulate)
{
    b.add_input<double>("Tolerance");
    b.add_input<Geometry>("G-IN");
    b.add_output<Geometry>("G-out");
}

NODE_EXECUTION_FUNCTION(triangulate)
{
    // Left empty.
    throw std::runtime_error("Not implemented!");
}

NODE_DECLARATION_UI(triangulate);
NODE_DEF_CLOSE_SCOPE
