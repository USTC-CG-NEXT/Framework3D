
#include "nodes/core/def/node_def.hpp"
#include "render_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(miss_sky)
{
    // Function content omitted
    b.add_input<BufferHandle>("SkyTexture");

}

NODE_EXECUTION_FUNCTION(miss_sky)
{
    // Function content omitted
    return true;
}

NODE_DECLARATION_UI(miss_sky);
NODE_DEF_CLOSE_SCOPE
