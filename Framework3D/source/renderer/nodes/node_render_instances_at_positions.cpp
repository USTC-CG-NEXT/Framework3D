
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
    // b.add_input<nvrhi::BufferHandle>("Transforms");

    b.add_output<nvrhi::TextureHandle>("Draw");
    b.add_output<nvrhi::TextureHandle>("Depth");
}

NODE_EXECUTION_FUNCTION(render_instances_at_positions)
{
    auto instance_name = params.get_input<std::string>("Instance");
    auto sdf_id = pxr::SdfPath(std::string(instance_name.c_str()));
    // auto transforms = params.get_input<nvrhi::BufferHandle>("Transforms");

    auto camera = get_free_camera(params);
    auto size = camera->dataWindow.GetSize();

    // Output texture
    nvrhi::TextureDesc desc =
        nvrhi::TextureDesc{}
            .setWidth(size[0])
            .setHeight(size[1])
            .setFormat(nvrhi::Format::RGBA8_UNORM)
            .setInitialState(nvrhi::ResourceStates::RenderTarget)
            .setKeepInitialState(true)
            .setIsRenderTarget(true);
    auto output_texture = resource_allocator.create(desc);

    // Depth texture
    nvrhi::TextureDesc depth_desc =
        nvrhi::TextureDesc{}
            .setWidth(size[0])
            .setHeight(size[1])
            .setFormat(nvrhi::Format::D32)
            .setIsRenderTarget(true)
            .setInitialState(nvrhi::ResourceStates::DepthWrite)
            .setKeepInitialState(true);
    auto depth_stencil_texture = resource_allocator.create(depth_desc);

    auto& meshes =
        params.get_global_payload<RenderGlobalPayload&>().get_meshes();

    ProgramDesc vs_program_desc;
    vs_program_desc.shaderType = nvrhi::ShaderType::Vertex;
    vs_program_desc.set_path("shaders/rasterize.slang")
        .set_entry_name("vs_main");

    ProgramHandle vs_program = resource_allocator.create(vs_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(vs_program);

    ProgramDesc ps_program_desc;
    ps_program_desc.shaderType = nvrhi::ShaderType::Pixel;
    ps_program_desc.set_path("shaders/rasterize.slang")
        .set_entry_name("ps_main");
    ProgramHandle ps_program = resource_allocator.create(ps_program_desc);
    MARK_DESTROY_NVRHI_RESOURCE(ps_program);

    auto view_cb = get_free_camera_cb(params);
    MARK_DESTROY_NVRHI_RESOURCE(view_cb);

    nvrhi::BufferDesc matrixBufferDesc =
        nvrhi::BufferDesc{}
            .setByteSize(sizeof(pxr::GfMatrix4f) * 10)
            .setStructStride(sizeof(pxr::GfMatrix4f))
            .setCanHaveUAVs(true)
            .setInitialState(nvrhi::ResourceStates::Common)
            .setCpuAccess(nvrhi::CpuAccessMode::Write)
            .setKeepInitialState(true);
    auto matrixBuffer = resource_allocator.create(matrixBufferDesc);
    MARK_DESTROY_NVRHI_RESOURCE(matrixBuffer);

    pxr::VtArray<pxr::GfMatrix4f> matricies;
    for (int i = 0; i < 10; ++i) {
        auto translation = pxr::GfVec3f(i * 2.0f, 0.0f, 0.0f);
        auto matrix = pxr::GfMatrix4f(1.0f);
        matrix.SetTranslate(translation);
        matricies.push_back(matrix);
    }

    auto matrices = resource_allocator.device->mapBuffer(
        matrixBuffer, nvrhi::CpuAccessMode::Write);
    memcpy(matrices, matricies.data(), sizeof(pxr::GfMatrix4f) * 10);
    resource_allocator.device->unmapBuffer(matrixBuffer);

    if (!vs_program->get_error_string().empty()) {
        log::warning(
            "Failed to create vertex shader program: %s",
            vs_program->get_error_string().c_str());

        resource_allocator.destroy(output_texture);
        resource_allocator.destroy(depth_stencil_texture);
        return false;
    }

    if (!ps_program->get_error_string().empty()) {
        log::warning(
            "Failed to create pixel shader program: %s",
            ps_program->get_error_string().c_str());
        resource_allocator.destroy(output_texture);
        resource_allocator.destroy(depth_stencil_texture);
        return false;
    }

    ProgramVars program_vars(resource_allocator, vs_program, ps_program);
    RenderContext context(resource_allocator, program_vars);

    context.set_render_target(0, output_texture)
        .set_depth_stencil_target(depth_stencil_texture)
        .finish_setting_frame_buffer()
        .add_vertex_buffer_desc(
            "POSITION",
            nvrhi::Format::RGB32_FLOAT,
            0,
            1,
            0,
            sizeof(pxr::GfVec3f),
            true)
        .add_vertex_buffer_desc(
            "NORMAL",
            nvrhi::Format::RGB32_FLOAT,
            1,
            1,
            0,
            sizeof(pxr::GfVec3f),
            true)
        .set_viewport(size)
        .finish_setting_pso();

    // find the named mesh
    for (Hd_USTC_CG_Mesh*& mesh : meshes) {
        //        if (mesh->GetId() == sdf_id)

        program_vars["modelMatrixBuffer"] = matrixBuffer;
        program_vars["viewConstant"] = view_cb;

        program_vars.finish_setting_vars();

        GraphicsRenderState state;

        state
            .addVertexBuffer(
                nvrhi::VertexBufferBinding{ mesh->GetVertexBuffer(), 0 ,0})
            .addVertexBuffer(
                nvrhi::VertexBufferBinding{ mesh->GetNormalBuffer(), 1,0 })
            .setIndexBuffer(nvrhi::IndexBufferBinding{
                mesh->GetIndexBuffer(), nvrhi::Format::R32_UINT ,0});

        // state.addVertexBuffer(nvrhi::VertexBufferBinding{ debugVertexBuffer,
        // 0, 0 })
        //     .addVertexBuffer(nvrhi::VertexBufferBinding{ debugNormalBuffer,
        //     1, 0 }) .setIndexBuffer(nvrhi::IndexBufferBinding{
        //         debugIndexBuffer, nvrhi::Format::R32_UINT, 0 });

        context.draw_instanced(state, program_vars, mesh->IndexCount(), 1u);
    }

    params.set_output("Draw", output_texture);
    params.set_output("Depth", depth_stencil_texture);
    return true;
}

NODE_DECLARATION_UI(render_instances_at_positions);
NODE_DEF_CLOSE_SCOPE
