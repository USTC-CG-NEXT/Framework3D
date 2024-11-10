#include "render_context.hpp"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/gf/vec3f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

RenderContext::RenderContext(ResourceAllocator& resource_allocator)
    : resource_allocator_(resource_allocator)
{
    commandList_ = resource_allocator_.create(CommandListDesc{});
}

RenderContext::~RenderContext()
{
    resource_allocator_.destroy(commandList_);
    resource_allocator_.destroy(framebuffer_);
}

RenderContext& RenderContext::set_program(const ProgramHandle& program)
{
    this->program_ = program.Get();
    return *this;
}

RenderContext& RenderContext::set_render_target(
    unsigned i,
    const TextureHandle& texture)
{
    if (i >= framebuffer_desc_.colorAttachments.size()) {
        framebuffer_desc_.colorAttachments.resize(i + 1);
    }

    framebuffer_desc_.colorAttachments[i].texture = texture;
    return *this;
}

void RenderContext::draw_instanced(
    const GraphicsRenderState& state,
    const ProgramVars& program_vars,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t startIndexLocation,
    int32_t baseVertexLocation,
    uint32_t startInstanceLocation)
{
    nvrhi::GraphicsState graphics_state;

    graphics_state.vertexBuffers = state.vertexBuffers;
    graphics_state.indexBuffer = state.indexBuffer;
    graphics_state.bindings = program_vars.get_binding_sets();
    graphics_state.framebuffer = framebuffer_;
    graphics_state.pipeline = graphics_pipeline;

    nvrhi::DrawArguments args;
    args.vertexCount = indexCount;
    args.instanceCount = instanceCount;
    args.startIndexLocation = startIndexLocation;
    args.startVertexLocation = baseVertexLocation;
    args.startInstanceLocation = startInstanceLocation;

    commandList_->open();
    commandList_->setGraphicsState(graphics_state);
    commandList_->drawIndexed(args);
    commandList_->close();
    resource_allocator_.device->executeCommandList(commandList_);
}

RenderContext& RenderContext::finish_setting_frame_buffer()
{
    if (framebuffer_) {
        resource_allocator_.destroy(framebuffer_);
        framebuffer_ = nullptr;
    }

    framebuffer_ = resource_allocator_.create(framebuffer_desc_);

    return *this;
}

nvrhi::VertexAttributeDesc RenderContext::GetVertexAttributeDesc(
    VertexAttribute attribute,
    const char* name,
    uint32_t bufferIndex)
{
    nvrhi::VertexAttributeDesc result = {};
    result.name = name;
    result.bufferIndex = bufferIndex;
    result.arraySize = 1;

    switch (attribute) {
        case VertexAttribute::Position:
        case VertexAttribute::PrevPosition:
            result.format = nvrhi::Format::RGB32_FLOAT;
            result.elementStride = sizeof(pxr::GfVec3f);
            break;
        case VertexAttribute::TexCoord1:
        case VertexAttribute::TexCoord2:
            result.format = nvrhi::Format::RG32_FLOAT;
            result.elementStride = sizeof(pxr::GfVec2f);
            break;
        case VertexAttribute::Normal:
        case VertexAttribute::Tangent:
            result.format = nvrhi::Format::RGBA8_SNORM;
            result.elementStride = sizeof(uint32_t);
            break;
            // case VertexAttribute::Transform:
            //     result.format = nvrhi::Format::RGBA32_FLOAT;
            //     result.arraySize = 3;
            //     result.offset = offsetof(InstanceData, transform);
            //     result.elementStride = sizeof(InstanceData);
            //     result.isInstanced = true;
            //     break;
            // case VertexAttribute::PrevTransform:
            //     result.format = nvrhi::Format::RGBA32_FLOAT;
            //     result.arraySize = 3;
            //     result.offset = offsetof(InstanceData, prevTransform);
            //     result.elementStride = sizeof(InstanceData);
            //     result.isInstanced = true;
            //     break;

        default: assert(!"unknown attribute");
    }

    return result;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE