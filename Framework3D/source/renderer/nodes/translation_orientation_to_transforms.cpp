
#include "nodes/core/def/node_def.hpp"
#include "render_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(translation_orientation_to_transforms)
{
    // Function content omitted
    b.add_input<nvrhi::TextureHandle>("World Position");
    b.add_input<nvrhi::TextureHandle>("World Direction");

    b.add_output<nvrhi::BufferHandle>("Transforms");
}

NODE_EXECUTION_FUNCTION(translation_orientation_to_transforms)
{

    // Function content omitted
    return true;
}

NODE_DECLARATION_UI(translation_orientation_to_transforms);
NODE_DEF_CLOSE_SCOPE
