
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/hgiGL/computeCmds.h"
#include "render_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(rasterize_impl)
{
    b.add_input<std::string>("Vertex Shader")
        .default_val("shaders/rasterize_impl.vs");
    b.add_input<std::string>("Fragment Shader")
        .default_val("shaders/rasterize_impl.fs");
    b.add_output<nvrhi::TextureHandle>("Position");
    b.add_output<nvrhi::TextureHandle>("Depth");
    b.add_output<nvrhi::TextureHandle>("Texcoords");
    b.add_output<nvrhi::TextureHandle>("diffuseColor");
    b.add_output<nvrhi::TextureHandle>("MetallicRoughness");
    b.add_output<nvrhi::TextureHandle>("Normal");
}

NODE_EXECUTION_FUNCTION(rasterize_impl)
{
    return true;
}

NODE_DECLARATION_UI(rasterize_impl);
NODE_DEF_CLOSE_SCOPE
