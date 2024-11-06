#include "render_node_base.h"

#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(scene_lights)
{

}

NODE_EXECUTION_FUNCTION(scene_lights)
{
    // Do nothing. The output is filled in renderer.
}



NODE_DECLARATION_UI(scene_lights);
NODE_DEF_CLOSE_SCOPE
