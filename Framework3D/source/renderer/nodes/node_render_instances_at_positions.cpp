
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "renderer/program_vars.hpp"
#include "renderer/render_context.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(render_instances_at_positions)
{
    b.add_input<std::string>("Instance");
    b.add_input<nvrhi::BufferHandle>("Transforms");

    b.add_output<nvrhi::TextureHandle>("Draw");
}

NODE_EXECUTION_FUNCTION(render_instances_at_positions)
{
    auto instance_name = params.get_input<std::string>("Instance");
    auto sdf_id = pxr::SdfPath(instance_name.c_str());
    auto transforms = params.get_input<nvrhi::BufferHandle>("Transforms");

    auto camera = get_free_camera(params);
    auto size = camera->dataWindow.GetSize();

    // Output texture
    nvrhi::TextureDesc desc =
        nvrhi::TextureDesc{}
            .setWidth(size[0])
            .setHeight(size[1])
            .setFormat(nvrhi::Format::RGBA8_UNORM)
            .setInitialState(nvrhi::ResourceStates::UnorderedAccess)
            .setKeepInitialState(true)
            .setIsUAV(true);

    auto output_texture = resource_allocator.create(desc);

    auto& meshes = params.get_global_payload<RenderGlobalPayload&>().meshes;

    // find the named mesh
    for (Hd_USTC_CG_Mesh*& mesh : meshes) {
        if (mesh->GetId() == sdf_id) {
            ProgramHandle program;

            ProgramVars program_vars(program, resource_allocator);

            program_vars["Name"] = nvrhi::BindingSetItem();

            program_vars.finish_setting_vars();

            RenderBufferState state;

            RenderContext context(resource_allocator);

            context.set_program(program)
                .set_render_target(0, output_texture)
                .finish_setting_frame_buffer();

            context.draw_instanced(state, program_vars, 10u, 10u, 0u, 0u, 0u);
        }
    }

    params.set_output("Draw", output_texture);
}

NODE_DECLARATION_UI(render_instances_at_positions);
NODE_DEF_CLOSE_SCOPE
