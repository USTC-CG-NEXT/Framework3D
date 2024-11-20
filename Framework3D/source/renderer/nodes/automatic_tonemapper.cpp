
#include "nodes/core/def/node_def.hpp"
#include "render_node_base.h"
#include "renderer/compute_context.hpp"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(automatic_tonemapper)
{
    b.add_input<nvrhi::TextureHandle>("InputTexture");
    b.add_input<float>("Scale").min(0).max(20).default_val(1);
    b.add_output<nvrhi::TextureHandle>("OutputTexture");
}

NODE_EXECUTION_FUNCTION(automatic_tonemapper)
{
    ProgramDesc cs_program_desc;
    cs_program_desc.shaderType = nvrhi::ShaderType::Compute;
    cs_program_desc.set_path("shaders/tonemapper.slang").set_entry_name("main");
    ProgramHandle cs_program = resource_allocator.create(cs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(cs_program);
    CHECK_PROGRAM_ERROR(cs_program);

    auto input_texture = params.get_input<nvrhi::TextureHandle>("InputTexture");

    auto desc = input_texture->getDesc();
    desc.isUAV = true;
    desc.initialState = nvrhi::ResourceStates::UnorderedAccess;
    desc.keepInitialState = true;
    auto output_texture = resource_allocator.create(desc);

    ProgramVars program_vars(resource_allocator, cs_program);
    program_vars["gInputTexture"] = input_texture;
    program_vars["gOutputTexture"] = output_texture;

    auto image_size = GfVec2i(
        input_texture->getDesc().width, input_texture->getDesc().height);
    auto size_cb = create_constant_buffer(params, image_size);
    MARK_DESTROY_NVRHI_RESOURCE(size_cb);

    float scale = params.get_input<float>("Scale");
    auto scale_cb = create_constant_buffer(params, scale);
    MARK_DESTROY_NVRHI_RESOURCE(scale_cb);

    program_vars["cbToneMappingParams"] = scale_cb;

    program_vars["cbImageSize"] = size_cb;

    program_vars.finish_setting_vars();

    ComputeContext context(resource_allocator, program_vars);

    context.finish_setting_pso();

    context.begin();
    context.dispatch({}, program_vars, image_size[0], 16, image_size[1], 16);
    context.finish();

    params.set_output("OutputTexture", output_texture);

    return true;
}

NODE_DECLARATION_UI(automatic_tonemapper);
NODE_DEF_CLOSE_SCOPE
