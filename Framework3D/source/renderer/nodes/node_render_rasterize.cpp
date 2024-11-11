
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/hgiGL/computeCmds.h"
#include "render_node_base.h"
#include "renderer/program_vars.hpp"
#include "renderer/render_context.hpp"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(rasterize)
{
    b.add_output<nvrhi::TextureHandle>("Position");
    b.add_output<nvrhi::TextureHandle>("Depth");
    b.add_output<nvrhi::TextureHandle>("Texcoords");
    b.add_output<nvrhi::TextureHandle>("DiffuseColor");
    b.add_output<nvrhi::TextureHandle>("MetallicRoughness");
    b.add_output<nvrhi::TextureHandle>("Normal");
}

NODE_EXECUTION_FUNCTION(rasterize)
{
    std::vector<ShaderMacro> macros{ { "ENABLE_DIFFUSE_COLOR", "1" },
                                     { "ENABLE_METALLIC_ROUGHNESS", "1" },
                                     { "ENABLE_NORMAL", "1" },
                                     { "ENABLE_TEXCOORD", "1" } };

    ProgramDesc vs_program_desc;
    vs_program_desc.shaderType = nvrhi::ShaderType::Vertex;
    vs_program_desc.set_path("shaders/rasterize.slang")
        .set_entry_name("vs_main")
        .define(macros);

    ProgramHandle vs_program = resource_allocator.create(vs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(vs_program);
    CHECK_PROGRAM_ERROR(vs_program);

    ProgramDesc ps_program_desc;
    ps_program_desc.shaderType = nvrhi::ShaderType::Pixel;
    ps_program_desc.set_path("shaders/rasterize.slang")
        .set_entry_name("ps_main")
        .define(macros);
    ProgramHandle ps_program = resource_allocator.create(ps_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(ps_program);
    CHECK_PROGRAM_ERROR(ps_program);

    auto view_cb = get_free_camera_cb(params);
    MARK_DESTROY_NVRHI_RESOURCE(view_cb);

    auto output_position = create_default_render_target(params);
    auto output_depth = create_default_depth_stencil(params);
    auto output_texcoords =
        create_default_render_target(params, nvrhi::Format::RG8_UNORM);
    auto output_diffuse_color = create_default_render_target(params);
    auto output_metallic_roughness = create_default_render_target(params);
    auto output_normal = create_default_render_target(params);

    ProgramVars program_vars(resource_allocator, vs_program, ps_program);
    program_vars["viewConstant"] = view_cb;

    RenderContext context(resource_allocator, program_vars);
    context.set_render_target(0, output_position)
        .set_render_target(1, output_texcoords)
        .set_render_target(2, output_diffuse_color)
        .set_render_target(3, output_metallic_roughness)
        .set_render_target(4, output_normal)
        .set_depth_stencil_target(output_depth)
        .finish_setting_frame_buffer();

    context
        .add_vertex_buffer_desc(
            "POSITION",
            nvrhi::Format::RGB32_FLOAT,
            0,
            1,
            0,
            sizeof(pxr::GfVec3f),
            false)
        .add_vertex_buffer_desc(
            "NORMAL",
            nvrhi::Format::RGB32_FLOAT,
            1,
            1,
            0,
            sizeof(pxr::GfVec3f),
            false)
        .add_vertex_buffer_desc(
            "TEXCOORD",
            nvrhi::Format::RG32_FLOAT,
            2,
            1,
            0,
            sizeof(pxr::GfVec2f),
            false)
        .set_viewport(get_size(params))
        .finish_setting_pso();

    // find the named mesh
    context.begin_render();

    auto& meshes =
        params.get_global_payload<RenderGlobalPayload&>().get_meshes();
    for (Hd_USTC_CG_Mesh*& mesh : meshes) {
        if (mesh->GetVertexBuffer()) {
            GraphicsRenderState state;
            state
                .addVertexBuffer(
                    nvrhi::VertexBufferBinding{ mesh->GetVertexBuffer(), 0, 0 })
                .addVertexBuffer(
                    nvrhi::VertexBufferBinding{ mesh->GetNormalBuffer(), 1, 0 })
                .setIndexBuffer(nvrhi::IndexBufferBinding{
                    mesh->GetIndexBuffer(), nvrhi::Format::R32_UINT, 0 });

            if (mesh->GetTexcoordBuffer(pxr::TfToken("UVMap"))) {
                state.addVertexBuffer(nvrhi::VertexBufferBinding{
                    mesh->GetTexcoordBuffer(pxr::TfToken("UVMap")), 2, 0 });
            }
            else {
                nvrhi::BufferDesc texcoord_buffer_desc = nvrhi::BufferDesc();
                texcoord_buffer_desc.byteSize =
                    mesh->PointCount() * sizeof(pxr::GfVec2f);
                texcoord_buffer_desc.isVertexBuffer = true;
                texcoord_buffer_desc.initialState =
                    nvrhi::ResourceStates::ShaderResource;
                texcoord_buffer_desc.debugName = "texcoordBuffer";
                texcoord_buffer_desc.cpuAccess = nvrhi::CpuAccessMode::Write;
                auto texcoord_buffer =
                    resource_allocator.create(texcoord_buffer_desc);

                MARK_DESTROY_NVRHI_RESOURCE(texcoord_buffer);

                // fill the buffer with default values
                std::vector<pxr::GfVec2f> texcoords(
                    mesh->PointCount(), { 0, 0 });
                auto ptr = resource_allocator.device->mapBuffer(
                    texcoord_buffer, nvrhi::CpuAccessMode::Write);

                memcpy(
                    ptr,
                    texcoords.data(),
                    texcoords.size() * sizeof(pxr::GfVec2f));

                resource_allocator.device->unmapBuffer(texcoord_buffer);

                state.addVertexBuffer(
                    nvrhi::VertexBufferBinding{ texcoord_buffer, 2, 0 });
            }

            auto model_matrix = get_model_buffer(params, mesh->GetTransform());
            MARK_DESTROY_NVRHI_RESOURCE(model_matrix);
            program_vars["modelMatrixBuffer"] = model_matrix;
            program_vars.finish_setting_vars();

            context.draw(state, program_vars, mesh->IndexCount());
        }
    }
    context.finish_render();

    params.set_output("Position", output_position);
    params.set_output("Depth", output_depth);
    params.set_output("Texcoords", output_texcoords);
    params.set_output("DiffuseColor", output_diffuse_color);
    params.set_output("MetallicRoughness", output_metallic_roughness);
    params.set_output("Normal", output_normal);

    return true;
}

NODE_DECLARATION_UI(rasterize);
NODE_DEF_CLOSE_SCOPE
