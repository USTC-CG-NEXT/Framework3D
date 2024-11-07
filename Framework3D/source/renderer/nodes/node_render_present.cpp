#include "nvrhi/nvrhi.h"
#include "render_node_base.h"

#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(present_color)
{
    b.add_input<nvrhi::TextureHandle>("Color");
}

NODE_DECLARATION_FUNCTION(present_depth)
{
    b.add_input<nvrhi::TextureHandle>("Depth");
}

NODE_EXECUTION_FUNCTION(present_color)
{
    // Do nothing. Wait for external statements to fetch
}

NODE_EXECUTION_FUNCTION(present_depth)
{
    // Do nothing. Wait for external statements to fetch
}


NODE_DEF_CLOSE_SCOPE
