#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "render_node_base.h"
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
    return true;
}

NODE_EXECUTION_FUNCTION(present_depth)
{
    // Do nothing. Wait for external statements to fetch
    return true;
}

NODE_DECLARATION_REQUIRED(present_color)
NODE_DECLARATION_REQUIRED(present_depth)

NODE_DEF_CLOSE_SCOPE
