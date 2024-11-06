#include "NODES_FILES_DIR.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "RCore/Backend.hpp"
#include "geometries/mesh.h"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "renderer/program_vars.hpp"
#include "renderer/render_context.hpp"
#include "resource_allocator_instance.hpp"

namespace USTC_CG::node_render_instances_at_positions {
static void node_declare(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Camera>("Camera");
    b.add_input<decl::String>("Instance");
    b.add_input<decl::Buffer>("Transforms");

    b.add_output<decl::Texture>("Draw");
}

static void node_exec(ExeParams params)
{
    auto instance_name = params.get_input<std::string>("Instance");
    auto sdf_id = pxr::SdfPath(instance_name.c_str());
    auto transforms = params.get_input<nvrhi::BufferHandle>("Transforms");

    auto camera = get_free_camera(params);
    auto size = camera->dataWindow.GetSize();

    // Output texture
    TextureDesc desc =
        TextureDesc{}
            .setWidth(size[0])
            .setHeight(size[1])
            .setFormat(nvrhi::Format::RGBA8_UNORM)
            .setInitialState(nvrhi::ResourceStates::UnorderedAccess)
            .setKeepInitialState(true)
            .setIsUAV(true);

    auto output_texture = resource_allocator.create(desc);

    auto meshes = params.get_input<MeshArray>("Meshes");

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

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "render_instances_at_positions");
    strcpy(ntype.id_name, "node_render_instances_at_positions");

    render_node_type_base(&ntype);
    ntype.node_execute = node_exec;
    ntype.declare = node_declare;
    nodeRegisterType(&ntype);
}


}  // namespace USTC_CG::node_render_instances_at_positions
