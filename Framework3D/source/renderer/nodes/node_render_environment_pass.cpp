
#include "nodes/core/def/node_def.hpp"
#include "pxr/base/gf/frustum.h"
#include "pxr/imaging/glf/simpleLight.h"
#include "render_node_base.h"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(environment_pass)
{
    b.add_input<nvrhi::TextureHandle>("Color");
    b.add_input<nvrhi::TextureHandle>("Depth");

    b.add_input<std::string>("Shader").default_val(
        "shaders/environment_map.fs");
    b.add_output<nvrhi::TextureHandle>("Color");
}

NODE_EXECUTION_FUNCTION(environment_pass)
{
    return true;
}

NODE_DECLARATION_UI(environment_pass);
NODE_DEF_CLOSE_SCOPE
