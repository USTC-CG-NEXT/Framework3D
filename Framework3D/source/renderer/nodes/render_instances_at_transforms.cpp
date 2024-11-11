
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "renderer/program_vars.hpp"
#include "renderer/render_context.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(render_instances_at_transforms)
{
    b.add_input<std::string>("Instance");
    b.add_input<nvrhi::BufferHandle>("Transforms");
    b.add_input<int>("Buffer Size").min(0).max(100).default_val(0);

    b.add_output<nvrhi::TextureHandle>("Position");
    b.add_output<nvrhi::TextureHandle>("Depth");
    b.add_output<nvrhi::TextureHandle>("Texcoords");
    b.add_output<nvrhi::TextureHandle>("DiffuseColor");
    b.add_output<nvrhi::TextureHandle>("MetallicRoughness");
    b.add_output<nvrhi::TextureHandle>("Normal");
}

NODE_EXECUTION_FUNCTION(render_instances_at_transforms)
{
    ProgramDesc vs_program_desc;
    vs_program_desc.shaderType = nvrhi::ShaderType::Vertex;
    vs_program_desc.set_path("shaders/rasterize.slang")
        .set_entry_name("vs_main");

    ProgramHandle vs_program = resource_allocator.create(vs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(vs_program);
    CHECK_PROGRAM_ERROR(vs_program);

    ProgramDesc ps_program_desc;
    ps_program_desc.shaderType = nvrhi::ShaderType::Pixel;
    ps_program_desc.set_path("shaders/rasterize.slang")
        .set_entry_name("ps_main");
    ProgramHandle ps_program = resource_allocator.create(ps_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(ps_program);
    CHECK_PROGRAM_ERROR(ps_program);

    auto instance_name = params.get_input<std::string>("Instance");
    auto sdf_id = pxr::SdfPath(std::string(instance_name.c_str()));
    auto transforms = params.get_input<nvrhi::BufferHandle>("Transforms");

    auto output_texture = create_default_render_target(params);
    auto normal_texture = create_default_render_target(params);
    auto texcoord_texture =
        create_default_render_target(params, nvrhi::Format::RG8_UNORM);
    auto diffuse_color_texture = create_default_render_target(params);
    auto metallic_roughness_texture = create_default_render_target(params);

    auto depth_stencil_texture = create_default_depth_stencil(params);

    auto view_cb = get_free_camera_cb(params);
    MARK_DESTROY_NVRHI_RESOURCE(view_cb);

    auto camera = get_free_camera(params);
    auto size = camera->dataWindow.GetSize();

    ProgramVars program_vars(resource_allocator, vs_program, ps_program);
    RenderContext context(resource_allocator, program_vars);

    context.set_render_target(0, output_texture)
        .set_render_target(1, texcoord_texture)
        .set_render_target(2, diffuse_color_texture)
        .set_render_target(3, metallic_roughness_texture)
        .set_render_target(4, normal_texture)
        .set_depth_stencil_target(depth_stencil_texture)
        .finish_setting_frame_buffer()
        .add_vertex_buffer_desc("POSITION", 0, nvrhi::Format::RGB32_FLOAT)
        .add_vertex_buffer_desc("NORMAL", 1, nvrhi::Format::RGB32_FLOAT)
        .set_viewport(size)
        .finish_setting_pso();

    auto instance_count =
        transforms->getDesc().byteSize / sizeof(pxr::GfMatrix4f);

    auto instance_count_from_input = params.get_input<int>("Buffer Size");
    if (instance_count_from_input) {
        instance_count = instance_count_from_input;
    }

    program_vars["modelMatrixBuffer"] = transforms;
    program_vars["viewConstant"] = view_cb;

    // find the named mesh
    context.begin();

    auto& meshes =
        params.get_global_payload<RenderGlobalPayload&>().get_meshes();
    for (Hd_USTC_CG_Mesh*& mesh : meshes) {
        if (mesh->GetId() == sdf_id)
            if (mesh->GetVertexBuffer()) {
                program_vars.finish_setting_vars();

                GraphicsRenderState state;

                state
                    .addVertexBuffer(nvrhi::VertexBufferBinding{
                        mesh->GetVertexBuffer(), 0, 0 })
                    .addVertexBuffer(nvrhi::VertexBufferBinding{
                        mesh->GetNormalBuffer(), 1, 0 })
                    .setIndexBuffer(nvrhi::IndexBufferBinding{
                        mesh->GetIndexBuffer(), nvrhi::Format::R32_UINT, 0 });

                context.draw_instanced(
                    state, program_vars, mesh->IndexCount(), instance_count);
            }
    }
    context.finish();

    params.set_output("Position", output_texture);
    params.set_output("Normal", normal_texture);
    params.set_output("Texcoords", texcoord_texture);
    params.set_output("DiffuseColor", diffuse_color_texture);
    params.set_output("MetallicRoughness", metallic_roughness_texture);

    params.set_output("Depth", depth_stencil_texture);
    return true;
}

NODE_DECLARATION_UI(render_instances_at_positions);
NODE_DEF_CLOSE_SCOPE
