
#include <diff_optics/diff_optics.hpp>

#include "nodes/core/def/node_def.hpp"
#include "render_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(physical_lens_raygen)
{
    // Function content omitted
        
    b.add_output<BufferHandle>("Rays");
    b.add_output<BufferHandle>("Pixel Target");
}

NODE_EXECUTION_FUNCTION(physical_lens_raygen)
{
    // Function content omitted
    return true;
}

NODE_DECLARATION_UI(physical_lens_raygen);
NODE_DEF_CLOSE_SCOPE
