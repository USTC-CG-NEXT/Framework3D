#pragma once
#include "RHI/ResourceManager/resource_allocator.hpp"
#include "context.hpp"
#include "nvrhi/nvrhi.h"
#include "program_vars.hpp"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct GraphicsRenderState {
    nvrhi::
        static_vector<nvrhi::VertexBufferBinding, nvrhi::c_MaxVertexAttributes>
            vertexBuffers;
    nvrhi::IndexBufferBinding indexBuffer;

    GraphicsRenderState& addVertexBuffer(
        const nvrhi::VertexBufferBinding& value)
    {
        vertexBuffers.push_back(value);
        return *this;
    }
    GraphicsRenderState& setIndexBuffer(const nvrhi::IndexBufferBinding& value)
    {
        indexBuffer = value;
        return *this;
    }
};

class HD_USTC_CG_API GraphicsContext : public GPUContext {
   public:
    explicit GraphicsContext(ResourceAllocator& r, ProgramVars& vars);
    ~GraphicsContext() override;

    GraphicsContext& set_render_target(
        unsigned i,
        const nvrhi::TextureHandle& texture);

    GraphicsContext& set_depth_stencil_target(
        const nvrhi::TextureHandle& texture);

    void draw(
        const GraphicsRenderState& state,
        const ProgramVars& program_vars,
        uint32_t indexCount,
        uint32_t startIndexLocation = 0,
        int32_t baseVertexLocation = 0);

    void draw_instanced(
        const GraphicsRenderState& state,
        const ProgramVars& program_vars,
        uint32_t indexCount,
        uint32_t instanceCount = 1,
        uint32_t startIndexLocation = 0,
        int32_t baseVertexLocation = 0,
        uint32_t startInstanceLocation = 0);

    void draw_indirect(
        const GraphicsRenderState& state,
        const ProgramVars& program_vars,
        nvrhi::IBuffer* indirect_buffer,
        uint32_t draw_count);

    GraphicsContext& finish_setting_frame_buffer();
    GraphicsContext& set_viewport(pxr::GfVec2f size);

    GraphicsContext& add_vertex_buffer_desc(
        std::string name,
        uint32_t bufferIndex = 0,
        nvrhi::Format format = nvrhi::Format::UNKNOWN,
        uint32_t elementStride = 0,
        uint32_t arraySize = 1,
        uint32_t offset = 0,
        bool isInstanced = false);

    GraphicsContext& finish_setting_pso();
    void begin() override;

   private:
    nvrhi::GraphicsPipelineDesc pipeline_desc;
    nvrhi::FramebufferDesc framebuffer_desc_;
    nvrhi::FramebufferHandle framebuffer_ = nullptr;

    nvrhi::GraphicsPipelineHandle graphics_pipeline;

    nvrhi::VertexBufferBinding vbufBinding;
    nvrhi::IndexBufferBinding ibufBinding;
    std::vector<nvrhi::VertexAttributeDesc> vertex_attributes_;
    nvrhi::InputLayoutHandle input_layout;
    nvrhi::ShaderHandle vs_shader;
    nvrhi::ShaderHandle ps_shader;
    nvrhi::ViewportState viewport;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE