
#include <cstring>

#include "Logger/Logger.h"
#include "RHI/internal/nvrhi_patch.hpp"
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "render_node_base.h"
#include "renderer/compute_context.hpp"
#include "shaders/shaders/utils/cpp_shader_macro.h"
#include "utils/math.h"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(scatter_contribution)
{
    b.add_input<nvrhi::BufferHandle>("PixelTarget");
    b.add_input<nvrhi::BufferHandle>("Eval");
    b.add_input<int>("Buffer Size").min(0).max(10000000).default_val(1000);
    b.add_input<nvrhi::TextureHandle>("Source Texture");

    b.add_output<nvrhi::TextureHandle>("Result Texture");
}

NODE_EXECUTION_FUNCTION(scatter_contribution)
{
    using namespace nvrhi;

    auto pixel_target_buffer = params.get_input<BufferHandle>("PixelTarget");
    auto eval_buffer = params.get_input<BufferHandle>("Eval");
    auto source_texture = params.get_input<TextureHandle>("Source Texture");
    unsigned length = params.get_input<int>("Buffer Size");
    if (length > 0) {
        ProgramDesc atomic_scatter_desc;
        atomic_scatter_desc.set_path("shaders/atomic_scatter.slang")
            .set_entry_name("main")
            .set_shader_type(ShaderType::Compute);

        auto atomic_scatter_program =
            resource_allocator.create(atomic_scatter_desc);
        MARK_DESTROY_NVRHI_RESOURCE(atomic_scatter_program);
        CHECK_PROGRAM_ERROR(atomic_scatter_program);

        auto source_desc = source_texture->getDesc();
        source_desc.setFormat(Format::R32_FLOAT);
        source_desc.dimension = TextureDimension::Texture2DArray;
        source_desc.arraySize = 4;

        auto textures = resource_allocator.create(source_desc);
        MARK_DESTROY_NVRHI_RESOURCE(textures);

        auto length_buffer = create_constant_buffer(params, length);
        MARK_DESTROY_NVRHI_RESOURCE(length_buffer);

        ProgramVars program_vars(  resource_allocator, atomic_scatter_program);
        program_vars["textures"] = textures;
        program_vars["inputColor"] = eval_buffer;
        program_vars["inputPixelID"] = pixel_target_buffer;
        program_vars["bufferLength"] = length_buffer;
        program_vars.finish_setting_vars();

        ComputeContext context(resource_allocator, program_vars);
        context.finish_setting_pso();
        {
            PROFILE_SCOPE(scatter_contribution_dispatch);
            context.begin();
            context.clear_texture(textures);
            context.uav_barrier(textures);

            context.dispatch({}, program_vars, length, 64);
            context.finish();
        }

        ProgramDesc add_desc;
        add_desc.set_path("shaders/add_scatter.slang")
            .set_entry_name("main")
            .set_shader_type(ShaderType::Compute);
        auto add_program = resource_allocator.create(add_desc);
        MARK_DESTROY_NVRHI_RESOURCE(add_program);
        CHECK_PROGRAM_ERROR(add_program);

        ProgramVars add_program_vars(resource_allocator, add_program);
        add_program_vars["textures"] = textures;
        add_program_vars["outputTexture"] = source_texture;
        add_program_vars.finish_setting_vars();

        ComputeContext add_context(resource_allocator, add_program_vars);
        add_context.finish_setting_pso();
        {
            PROFILE_SCOPE(scatter_contribution_dispatch);
            add_context.begin();
            add_context.uav_barrier(textures);

            auto width = source_texture->getDesc().width;
            auto height = source_texture->getDesc().height;

            add_context.dispatch({}, add_program_vars, width, 16, height, 16);
            add_context.finish();
        }
    }
    else {
        log::warning("Buffer size is 0");
    }
    params.set_output("Result Texture", source_texture);
    return true;
}

NODE_DECLARATION_UI(scatter_contribution);
NODE_DEF_CLOSE_SCOPE
