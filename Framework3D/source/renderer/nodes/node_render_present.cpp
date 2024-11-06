#include "nvrhi/nvrhi.h"
#include "render_node_base.h"

#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(scene_present)
{
    b.add_input<nvrhi::TextureHandle>("Color");
}

NODE_DECLARATION_FUNCTION(scene_present)
{
    b.add_input<nvrhi::TextureHandle>("Depth");
}

NODE_EXECUTION_FUNCTION(scene_present)
{
    // Do nothing. Wait for external statements to fetch
}



NODE_DECLARATION_UI(scene_present);
NODE_DEF_CLOSE_SCOPE
