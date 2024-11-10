#include "render_context.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

RenderContext::RenderContext(ResourceAllocator& resource_allocator)
    : resource_allocator_(resource_allocator)
{
    commandList_ = resource_allocator_.create(CommandListDesc{});
}

RenderContext::~RenderContext()
{
    resource_allocator_.destroy(commandList_);
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
    state_.vertexBuffers = state.vertexBuffers;
    state_.indexBuffer = state.indexBuffer;

    state_.bindings = program_vars.get_binding_sets();

    nvrhi::DrawArguments args;
    args.vertexCount = indexCount;
    args.instanceCount = instanceCount;
    args.startIndexLocation = startIndexLocation;
    args.startVertexLocation = baseVertexLocation;
    args.startInstanceLocation = startInstanceLocation;

    commandList_->setGraphicsState(state_);

    commandList_->open();

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

USTC_CG_NAMESPACE_CLOSE_SCOPE