#pragma once

#include "RHI/ResourceManager/resource_allocator.hpp"
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

class RenderContext {
   public:
    explicit RenderContext(ResourceAllocator& r, ProgramVars& vars);
    ~RenderContext();

    RenderContext& set_render_target(
        unsigned i,
        const nvrhi::TextureHandle& texture);

    RenderContext& set_depth_stencil_target(
        const nvrhi::TextureHandle& texture);

    void draw_instanced(
        const GraphicsRenderState& state,
        const ProgramVars& program_vars,
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t startIndexLocation = 0,
        int32_t baseVertexLocation = 0,
        uint32_t startInstanceLocation = 0);

    RenderContext& finish_setting_frame_buffer();
    RenderContext& set_viewport(pxr::GfVec2f size);

    RenderContext& add_vertex_buffer_desc(
        std::string name,
        nvrhi::Format format = nvrhi::Format::UNKNOWN,
        uint32_t bufferIndex = 0,
        uint32_t arraySize = 1,
        uint32_t offset = 0,
        uint32_t elementStride = 0,
        bool isInstanced = false);

    RenderContext& finish_setting_pso();

    enum class VertexAttribute {
        Position,
        PrevPosition,
        TexCoord1,
        TexCoord2,
        Normal,
        Tangent,
        Transform,
        PrevTransform,
        JointIndices,
        JointWeights,

        Count
    };

   private:
    nvrhi::GraphicsPipelineDesc pipeline_desc;
    nvrhi::FramebufferDesc framebuffer_desc_;
    nvrhi::FramebufferHandle framebuffer_ = nullptr;

    ResourceAllocator& resource_allocator_;
    nvrhi::CommandListHandle commandList_;

    nvrhi::GraphicsPipelineHandle graphics_pipeline;

    nvrhi::VertexBufferBinding vbufBinding;
    nvrhi::IndexBufferBinding ibufBinding;
    std::vector<nvrhi::VertexAttributeDesc> vertex_attributes_;
    nvrhi::InputLayoutHandle input_layout;
    nvrhi::ShaderHandle vs_shader;
    nvrhi::ShaderHandle ps_shader;
    ProgramVars& vars_;
    nvrhi::ViewportState viewport;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE