#include "graphics_context.hpp"

#include "nvrhi/utils.h"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/gf/vec3f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

GraphicsContext::GraphicsContext(
    ResourceAllocator& resource_allocator,
    ProgramVars& vars)
    : GPUContext(resource_allocator, vars)
{
    auto programs = vars.get_programs();

    for (auto program : programs) {
        if (program->get_shader_desc().shaderType ==
            nvrhi::ShaderType::Vertex) {
            nvrhi::ShaderDesc vs_desc = program->get_shader_desc();
            vs_shader = resource_allocator_.create(
                vs_desc, program->getBufferPointer(), program->getBufferSize());
            pipeline_desc.VS = vs_shader;
        }
        else if (
            program->get_shader_desc().shaderType == nvrhi::ShaderType::Pixel) {
            nvrhi::ShaderDesc ps_desc = program->get_shader_desc();
            ps_shader = resource_allocator_.create(
                ps_desc, program->getBufferPointer(), program->getBufferSize());
            pipeline_desc.PS = ps_shader;
        }
    }
}

GraphicsContext::~GraphicsContext()
{
    resource_allocator_.destroy(framebuffer_);
    resource_allocator_.destroy(graphics_pipeline);
    resource_allocator_.destroy(vs_shader);
    resource_allocator_.destroy(ps_shader);
}

GraphicsContext& GraphicsContext::set_render_target(
    unsigned i,
    const TextureHandle& texture)
{
    if (i >= framebuffer_desc_.colorAttachments.size()) {
        framebuffer_desc_.colorAttachments.resize(i + 1);
    }

    framebuffer_desc_.colorAttachments[i].texture = texture;
    framebuffer_desc_.colorAttachments[i].format = texture->getDesc().format;

    return *this;
}

GraphicsContext& GraphicsContext::set_depth_stencil_target(
    const nvrhi::TextureHandle& texture)
{
    framebuffer_desc_.depthAttachment.texture = texture;
    return *this;
}

void GraphicsContext::draw(
    const GraphicsRenderState& state,
    const ProgramVars& program_vars,
    uint32_t indexCount,
    uint32_t startIndexLocation,
    int32_t baseVertexLocation)
{
    draw_instanced(
        state,
        program_vars,
        indexCount,
        1,
        startIndexLocation,
        baseVertexLocation,
        0);
}

void GraphicsContext::draw_instanced(
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
    graphics_state.viewport = viewport;

    commandList_->setGraphicsState(graphics_state);

    nvrhi::DrawArguments args;
    args.vertexCount = indexCount;
    args.instanceCount = instanceCount;
    args.startIndexLocation = startIndexLocation;
    args.startVertexLocation = baseVertexLocation;
    args.startInstanceLocation = startInstanceLocation;

    commandList_->drawIndexed(args);
}

void GraphicsContext::draw_indirect(
    const GraphicsRenderState& state,
    const ProgramVars& program_vars,
    nvrhi::IBuffer* indirect_buffer,
    uint32_t draw_count)
{
    if (draw_count == 0) {
        return;
    }
    nvrhi::GraphicsState graphics_state;

    graphics_state.vertexBuffers = state.vertexBuffers;
    graphics_state.indexBuffer = state.indexBuffer;
    graphics_state.bindings = program_vars.get_binding_sets();
    graphics_state.framebuffer = framebuffer_;
    graphics_state.pipeline = graphics_pipeline;
    graphics_state.viewport = viewport;
    graphics_state.indirectParams = indirect_buffer;

    commandList_->setGraphicsState(graphics_state);

    commandList_->drawIndexedIndirect(0, draw_count);
}

GraphicsContext& GraphicsContext::finish_setting_frame_buffer()
{
    if (framebuffer_) {
        resource_allocator_.destroy(framebuffer_);
        framebuffer_ = nullptr;
    }

    framebuffer_ = resource_allocator_.create(framebuffer_desc_);

    return *this;
}

GraphicsContext& GraphicsContext::set_viewport(pxr::GfVec2f size)
{
    viewport.scissorRects.resize(1);
    viewport.scissorRects[0].maxX = size[0];
    viewport.scissorRects[0].maxY = size[1];
    viewport.viewports.resize(1);
    viewport.viewports[0].maxX = size[0];
    viewport.viewports[0].maxY = size[1];

    return *this;
}

GraphicsContext& GraphicsContext::add_vertex_buffer_desc(
    std::string name,
    uint32_t bufferIndex,
    nvrhi::Format format,
    uint32_t elementStride,
    uint32_t arraySize,
    uint32_t offset,
    bool isInstanced)
{
    if (elementStride == 0) {
        nvrhi::FormatInfo formatInfo = getFormatInfo(format);
        elementStride = formatInfo.bytesPerBlock * formatInfo.blockSize;
    }

    vertex_attributes_.push_back(nvrhi::VertexAttributeDesc{}
                                     .setName(name.c_str())
                                     .setFormat(format)
                                     .setArraySize(arraySize)
                                     .setBufferIndex(bufferIndex)
                                     .setOffset(offset)
                                     .setElementStride(elementStride)
                                     .setIsInstanced(isInstanced));
    return *this;
}

GraphicsContext& GraphicsContext::finish_setting_pso()
{
    input_layout = resource_allocator_.device->createInputLayout(
        vertex_attributes_.data(), vertex_attributes_.size(), vs_shader);

    pipeline_desc.inputLayout = input_layout;

    nvrhi::BindingLayoutVector bindingLayouts = vars_.get_binding_layout();

    pipeline_desc.bindingLayouts = bindingLayouts;

    auto rasterState = nvrhi::RasterState().setFillSolid().setCullNone();

    auto depthStencilState = nvrhi::DepthStencilState()
                                 .enableDepthWrite()
                                 .enableDepthTest()
                                 .setDepthFunc(nvrhi::ComparisonFunc::Less);

    nvrhi::RenderState renderState;
    renderState.depthStencilState = depthStencilState;
    renderState.rasterState = rasterState;

    pipeline_desc.renderState = renderState;

    if (!framebuffer_) {
        log::error("framebuffer must be set before pso");
    }
    graphics_pipeline =
        resource_allocator_.create(pipeline_desc, framebuffer_.Get());

    return *this;
}

void GraphicsContext::begin()
{
    commandList_->open();
    commandList_->clearDepthStencilTexture(
        framebuffer_desc_.depthAttachment.texture, {}, true, 1.0f, false, 0);

    for (int i = 0; i < framebuffer_desc_.colorAttachments.size(); ++i) {
        nvrhi::utils::ClearColorAttachment(
            commandList_, framebuffer_, i, nvrhi::Color(0));
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE