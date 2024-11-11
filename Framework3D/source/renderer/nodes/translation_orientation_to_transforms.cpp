
#include "nodes/core/def/node_def.hpp"
#include "render_node_base.h"
#include "renderer/compute_context.hpp"
#include "renderer/program_vars.hpp"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(translation_orientation_to_transforms)
{
    // Function content omitted
    b.add_input<nvrhi::TextureHandle>("World Position");
    b.add_input<nvrhi::TextureHandle>("World Direction");

    b.add_output<nvrhi::BufferHandle>("Transforms");
    b.add_output<int>("Buffer Size");
}

NODE_EXECUTION_FUNCTION(translation_orientation_to_transforms)
{
    ProgramDesc cs_program_desc =
        ProgramDesc()
            .set_path("shaders/translation_orientation_to_transforms.slang")
            .set_entry_name("main")
            .set_shader_type(nvrhi::ShaderType::Compute);

    auto cs_program = resource_allocator.create(cs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(cs_program);
    CHECK_PROGRAM_ERROR(cs_program);

    auto world_position =
        params.get_input<nvrhi::TextureHandle>("World Position");
    auto world_direction =
        params.get_input<nvrhi::TextureHandle>("World Direction");

    ProgramVars program_vars(resource_allocator, cs_program);
    program_vars["WorldPosition"] = world_position;
    program_vars["WorldDirection"] = world_direction;
    auto size = get_size(params);
    auto size_buffer = create_constant_buffer(params, size);
    MARK_DESTROY_NVRHI_RESOURCE(size_buffer);
    program_vars["size"] = size_buffer;

    ComputeContext context(resource_allocator, program_vars);

    auto [output_buffer, counter] =
        create_counter_buffer<pxr::GfMatrix4f>(params, size[0] * size[1]);
    MARK_DESTROY_NVRHI_RESOURCE(counter);

    program_vars["Transforms"] = output_buffer;
    program_vars["Counter"] = counter;

    program_vars.finish_setting_vars();

    context.finish_setting_pso();

    context.begin();
    context.dispatch({}, program_vars, size[0], 32, size[1], 32);
    context.finish();

    int buffer_size = counter_read_out(params, counter);
    log::info("Buffer size: %d", buffer_size);

    params.set_output("Transforms", output_buffer);
    params.set_output("Buffer Size", buffer_size);

    return true;
}

NODE_DECLARATION_UI(translation_orientation_to_transforms);
NODE_DEF_CLOSE_SCOPE
