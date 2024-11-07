#pragma once

#include "RHI/ResourceManager/resource_allocator.hpp"
#include "nvrhi/nvrhi.h"
#include "program_vars.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

struct RenderBufferState {
    nvrhi::
        static_vector<nvrhi::VertexBufferBinding, nvrhi::c_MaxVertexAttributes>
            vertexBuffers;
    nvrhi::IndexBufferBinding indexBuffer;

    RenderBufferState& addVertexBuffer(const nvrhi::VertexBufferBinding& value)
    {
        vertexBuffers.push_back(value);
        return *this;
    }
    RenderBufferState& setIndexBuffer(const nvrhi::IndexBufferBinding& value)
    {
        indexBuffer = value;
        return *this;
    }
};

class RenderContext {
   public:
    explicit RenderContext(ResourceAllocator& r);
    ~RenderContext();
    RenderContext& set_program(const ProgramHandle& program);
    RenderContext& set_render_target(
        unsigned i,
        const nvrhi::TextureHandle& texture);
    void draw_instanced(
        const RenderBufferState& state,
        const ProgramVars& program_vars,
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t startIndexLocation,
        int32_t baseVertexLocation,
        uint32_t startInstanceLocation);
    RenderContext& finish_setting_frame_buffer();

   private:
    nvrhi::GraphicsState state_;

    nvrhi::FramebufferDesc framebuffer_desc_;
    nvrhi::FramebufferHandle framebuffer_ = nullptr;

    ResourceAllocator& resource_allocator_;
    nvrhi::CommandListHandle commandList_;
    IProgram* program_;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE