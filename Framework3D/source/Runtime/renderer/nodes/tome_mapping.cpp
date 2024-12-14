
#include "nodes/core/def/node_def.hpp"
#include "render_node_base.h"
#include "renderer/compute_context.hpp"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(tone_mapping)
{
    // Function content omitted
    b.add_input<nvrhi::TextureHandle>("Texture");
    b.add_output<nvrhi::TextureHandle>("ToneMapped");
}

NODE_EXECUTION_FUNCTION(tone_mapping)
{
    ProgramDesc cs_program_desc;
    cs_program_desc.shaderType = nvrhi::ShaderType::Compute;
    cs_program_desc.set_path("shaders/utils/ToneMapping.slang").set_entry_name("main");
    ProgramHandle cs_program = resource_allocator.create(cs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(cs_program);
    CHECK_PROGRAM_ERROR(cs_program);

    auto texture = params.get_input<nvrhi::TextureHandle>("Texture");

    ProgramVars program_vars(resource_allocator, cs_program);
    program_vars["SourceTexture"] = texture;

    auto mapped = resource_allocator.create(texture->getDesc());
    program_vars["MappedTexture"] = mapped;

    auto image_size = GfVec2i(texture->getDesc().width, texture->getDesc().height);
    auto size_cb = create_constant_buffer(params, image_size);
    MARK_DESTROY_NVRHI_RESOURCE(size_cb);

    program_vars["ImageSize"] = size_cb;

    program_vars.finish_setting_vars();

    ComputeContext context(resource_allocator, program_vars);

    context.finish_setting_pso();

    context.begin();
    context.dispatch({}, program_vars, image_size[0], 8, image_size[1], 8);
    context.finish();

    params.set_output("ToneMapped", mapped);
}

NODE_DECLARATION_UI(tone_mapping);
NODE_DEF_CLOSE_SCOPE
