
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
            .setFormat(nvrhi::Format::RGBA32_FLOAT)
            .setInitialState(nvrhi::ResourceStates::RenderTarget)
            .setKeepInitialState(true)
            .setIsRenderTarget(true)
            .setIsUAV(true);
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

    ProgramVars program_vars(resource_allocator, vs_program, ps_program);
    RenderContext context(resource_allocator, program_vars);

    context.set_render_target(0, output_texture)
        .set_depth_stencil_target(depth_stencil_texture)
        .finish_setting_frame_buffer()
        .add_vertex_buffer_desc("POSITION", nvrhi::Format::RGB32_FLOAT, 0)
        .add_vertex_buffer_desc("NORMAL", nvrhi::Format::RGB32_FLOAT, 1)
        .set_viewport(size)
        .finish_setting_pso();

    // Debug vertex buffer
    std::vector<pxr::GfVec3f> debugVertices = {
        pxr::GfVec3f(-1.0f, -1.0f, 0.0f),
        pxr::GfVec3f(1.0f, -1.0f, 0.0f),
        pxr::GfVec3f(1.0f, 1.0f, 0.0f),
        pxr::GfVec3f(-1.0f, 1.0f, 0.0f)
    };

    nvrhi::BufferDesc vertexBufferDesc =
        nvrhi::BufferDesc{}
            .setByteSize(sizeof(pxr::GfVec3f) * debugVertices.size())
            .setStructStride(sizeof(pxr::GfVec3f))
            .setCanHaveUAVs(false)
            .setInitialState(nvrhi::ResourceStates::VertexBuffer)
            .setCpuAccess(nvrhi::CpuAccessMode::Write)
            .setIsVertexBuffer(true)
            .setKeepInitialState(true);
    auto debugVertexBuffer = resource_allocator.create(vertexBufferDesc);
    MARK_DESTROY_NVRHI_RESOURCE(debugVertexBuffer);

    auto vertexData = resource_allocator.device->mapBuffer(
        debugVertexBuffer, nvrhi::CpuAccessMode::Write);
    memcpy(
        vertexData,
        debugVertices.data(),
        sizeof(pxr::GfVec3f) * debugVertices.size());
    resource_allocator.device->unmapBuffer(debugVertexBuffer);

    // Debug index buffer
    std::vector<uint32_t> debugIndices = { 0, 1, 2, 2, 3, 0 };

    nvrhi::BufferDesc indexBufferDesc =
        nvrhi::BufferDesc{}
            .setByteSize(sizeof(uint32_t) * debugIndices.size())
            .setStructStride(sizeof(uint32_t))
            .setCanHaveUAVs(false)
            .setInitialState(nvrhi::ResourceStates::IndexBuffer)
            .setCpuAccess(nvrhi::CpuAccessMode::Write)
            .setIsIndexBuffer(true)
            .setKeepInitialState(true);
    auto debugIndexBuffer = resource_allocator.create(indexBufferDesc);
    MARK_DESTROY_NVRHI_RESOURCE(debugIndexBuffer);

    auto indexData = resource_allocator.device->mapBuffer(
        debugIndexBuffer, nvrhi::CpuAccessMode::Write);
    memcpy(
        indexData, debugIndices.data(), sizeof(uint32_t) * debugIndices.size());
    resource_allocator.device->unmapBuffer(debugIndexBuffer);

    // Debug normal buffer
    std::vector<pxr::GfVec3f> debugNormals = { pxr::GfVec3f(0.0f, 0.0f, 1.0f),
                                               pxr::GfVec3f(0.0f, 0.0f, 1.0f),
                                               pxr::GfVec3f(0.0f, 0.0f, 1.0f),
                                               pxr::GfVec3f(0.0f, 0.0f, 1.0f) };

    nvrhi::BufferDesc normalBufferDesc =
        nvrhi::BufferDesc{}
            .setByteSize(sizeof(pxr::GfVec3f) * debugNormals.size())
            .setStructStride(sizeof(pxr::GfVec3f))
            .setCanHaveUAVs(false)
            .setInitialState(nvrhi::ResourceStates::VertexBuffer)
            .setCpuAccess(nvrhi::CpuAccessMode::Write)
            .setIsVertexBuffer(true)
            .setKeepInitialState(true);
    auto debugNormalBuffer = resource_allocator.create(normalBufferDesc);
    MARK_DESTROY_NVRHI_RESOURCE(debugNormalBuffer);

    auto normalData = resource_allocator.device->mapBuffer(
        debugNormalBuffer, nvrhi::CpuAccessMode::Write);
    memcpy(
        normalData,
        debugNormals.data(),
        sizeof(pxr::GfVec3f) * debugNormals.size());
    resource_allocator.device->unmapBuffer(debugNormalBuffer);

    // find the named mesh
    for (Hd_USTC_CG_Mesh*& mesh : meshes) {
        // if (mesh->GetId() == sdf_id)
        {
            program_vars["modelMatrixBuffer"] = matrixBuffer;
            program_vars["viewConstant"] = view_cb;

            program_vars.finish_setting_vars();

            GraphicsRenderState state;

            // state
            //     .addVertexBuffer(
            //         nvrhi::VertexBufferBinding{ mesh->GetVertexBuffer(), 0 })
            //     .addVertexBuffer(
            //         nvrhi::VertexBufferBinding{ mesh->GetNormalBuffer(), 1 })
            //     .setIndexBuffer(nvrhi::IndexBufferBinding{
            //         mesh->GetIndexBuffer(), nvrhi::Format::R32_UINT });

            state
                .addVertexBuffer(
                    nvrhi::VertexBufferBinding{ debugVertexBuffer, 0 })
                .addVertexBuffer(
                    nvrhi::VertexBufferBinding{ debugNormalBuffer, 1 })
                .setIndexBuffer(nvrhi::IndexBufferBinding{
                    debugIndexBuffer, nvrhi::Format::R32_UINT });

            context.draw_instanced(state, program_vars, debugIndices.size(), 10u);
        }
    }

    params.set_output("Draw", output_texture);
    params.set_output("Depth", depth_stencil_texture);
    return true;
}

NODE_DECLARATION_UI(render_instances_at_positions);
NODE_DEF_CLOSE_SCOPE
