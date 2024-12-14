#include "RHI/internal/resources.hpp"
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "render_node_base.h"
#include "shaders/shaders/utils/blit_cb.h"

NODE_DEF_OPEN_SCOPE
enum class BlitSampler { Point, Linear, Sharpen };

struct BlitParameters {
    static constexpr bool has_storage = false;

    nvrhi::Viewport targetViewport;

    pxr::GfRange2f targetBox = pxr::GfRange2f({ 0.f, 0.f }, { 1.f, 1.f });

    uint32_t sourceArraySlice = 0;
    uint32_t sourceMip = 0;
    pxr::GfRange2f sourceBox = pxr::GfRange2f({ 0.f, 0.f }, { 1.f, 1.f });

    BlitSampler sampler = BlitSampler::Linear;
    nvrhi::BlendState::RenderTarget blendState;
    nvrhi::Color blendConstantColor = nvrhi::Color(0.f);
};

NODE_DECLARATION_FUNCTION(blit_to_present)
{
    b.add_input<nvrhi::TextureHandle>("Tex");
    b.add_output<nvrhi::TextureHandle>("Tex");
}

static bool IsSupportedBlitDimension(nvrhi::TextureDimension dimension)
{
    return dimension == nvrhi::TextureDimension::Texture2D ||
           dimension == nvrhi::TextureDimension::Texture2DArray ||
           dimension == nvrhi::TextureDimension::TextureCube ||
           dimension == nvrhi::TextureDimension::TextureCubeArray;
}

static bool IsTextureArray(nvrhi::TextureDimension dimension)
{
    return dimension == nvrhi::TextureDimension::Texture2DArray ||
           dimension == nvrhi::TextureDimension::TextureCube ||
           dimension == nvrhi::TextureDimension::TextureCubeArray;
}

NODE_EXECUTION_FUNCTION(blit_to_present)
{
    auto sourceTexture = params.get_input<TextureHandle>("Tex");
    if (!sourceTexture) {
        log::warning("No texture to blit");
        return false;
    }
    auto output_desc = sourceTexture->getDesc();
    output_desc.format = nvrhi::Format::RGBA32_FLOAT;
    output_desc.isRenderTarget = true;
    auto output = resource_allocator.create(output_desc);
    MARK_DESTROY_NVRHI_RESOURCE(output);

    auto& blit_parameters = params.get_storage<BlitParameters&>();

    auto commandList = resource_allocator.create(CommandListDesc{});
    MARK_DESTROY_NVRHI_RESOURCE(commandList);

    assert(commandList);

    auto framebuffer_desc = nvrhi::FramebufferDesc{};

    framebuffer_desc.colorAttachments.push_back(
        nvrhi::FramebufferAttachment{ output.Get() });

    auto targetFramebuffer = resource_allocator.create(framebuffer_desc);
    MARK_DESTROY_NVRHI_RESOURCE(targetFramebuffer);

    const nvrhi::FramebufferDesc& targetFramebufferDesc =
        targetFramebuffer->getDesc();
    assert(targetFramebufferDesc.colorAttachments.size() == 1);
    assert(targetFramebufferDesc.colorAttachments[0].valid());
    assert(!targetFramebufferDesc.depthAttachment.valid());

    const nvrhi::FramebufferInfoEx& fbinfo =
        targetFramebuffer->getFramebufferInfo();
    const nvrhi::TextureDesc& sourceDesc = sourceTexture->getDesc();

    assert(IsSupportedBlitDimension(sourceDesc.dimension));
    bool isTextureArray = IsTextureArray(sourceDesc.dimension);

    nvrhi::Viewport targetViewport = blit_parameters.targetViewport;
    if (targetViewport.width() == 0 && targetViewport.height() == 0) {
        // If no viewport is specified, create one based on the framebuffer
        // dimensions. Note that the FB dimensions may not be the same as target
        // texture dimensions, in case a non-zero mip level is used.
        targetViewport =
            nvrhi::Viewport(float(fbinfo.width), float(fbinfo.height));
    }

    std::vector macro_defines = { ShaderMacro("TEXTURE_ARRAY", "0") };

    std::string vs_name = "rect_vs.hlsl";
    std::string ps_name = "blit_ps.hlsl";

    std::string error_string;

    ShaderReflectionInfo vs_reflection_info;
    auto vertex_shader = shader_factory.compile_shader(
        "main",
        nvrhi::ShaderType::Vertex,
        "shaders/utils/" + vs_name,
        vs_reflection_info,
        error_string,
        macro_defines);
    MARK_DESTROY_NVRHI_RESOURCE(vertex_shader);

    ShaderReflectionInfo ps_reflection_info;

    auto pixel_shader = shader_factory.compile_shader(
        "main",
        nvrhi::ShaderType::Pixel,
        "shaders/utils/" + ps_name,
        ps_reflection_info,
        error_string,
        macro_defines);
    MARK_DESTROY_NVRHI_RESOURCE(pixel_shader);

    if (!error_string.empty()) {
        log::warning(error_string.c_str());
        return false;
    }

    nvrhi::BindingLayoutDescVector binding_layout_descs =
        (vs_reflection_info + ps_reflection_info).get_binding_layout_descs();

    auto samplerDesc =
        nvrhi::SamplerDesc().setAllFilters(false).setAllAddressModes(
            nvrhi::SamplerAddressMode::Clamp);
    auto m_PointClampSampler = resource_allocator.create(samplerDesc);
    MARK_DESTROY_NVRHI_RESOURCE(m_PointClampSampler);

    samplerDesc.setAllFilters(true);
    auto m_LinearClampSampler = resource_allocator.create(samplerDesc);
    MARK_DESTROY_NVRHI_RESOURCE(m_LinearClampSampler);

    auto binding_layout = resource_allocator.create(binding_layout_descs[0]);
    MARK_DESTROY_NVRHI_RESOURCE(binding_layout);

    nvrhi::GraphicsPipelineDesc psoDesc;
    psoDesc.bindingLayouts = { binding_layout };
    psoDesc.VS = vertex_shader;
    psoDesc.PS = pixel_shader;
    psoDesc.primType = nvrhi::PrimitiveType::TriangleStrip;
    psoDesc.renderState.rasterState.setCullNone();
    psoDesc.renderState.depthStencilState.depthTestEnable = false;
    psoDesc.renderState.depthStencilState.stencilEnable = false;
    psoDesc.renderState.blendState.targets[0] = blit_parameters.blendState;

    auto constant_buffer = resource_allocator.create(
        BufferDesc{ .byteSize = sizeof(BlitParameters),
                    .debugName = "BlitParameters",
                    .isConstantBuffer = true,
                    .initialState = nvrhi::ResourceStates::ConstantBuffer,
                    .cpuAccess = nvrhi::CpuAccessMode::Write });
    MARK_DESTROY_NVRHI_RESOURCE(constant_buffer);

    auto pso = resource_allocator.create(psoDesc, targetFramebuffer);
    MARK_DESTROY_NVRHI_RESOURCE(pso);

    BindingSetDesc binding_set_desc;
    {
        auto sourceDimension = sourceDesc.dimension;
        if (sourceDimension == nvrhi::TextureDimension::TextureCube ||
            sourceDimension == nvrhi::TextureDimension::TextureCubeArray)
            sourceDimension = nvrhi::TextureDimension::Texture2DArray;

        auto sourceSubresources = nvrhi::TextureSubresourceSet(
            blit_parameters.sourceMip, 1, blit_parameters.sourceArraySlice, 1);

        binding_set_desc.bindings = {
            nvrhi::BindingSetItem::ConstantBuffer(0, constant_buffer),
            nvrhi::BindingSetItem::Sampler(
                0,
                blit_parameters.sampler == BlitSampler::Point
                    ? m_PointClampSampler
                    : m_LinearClampSampler),
            nvrhi::BindingSetItem::Texture_SRV(
                0,
                sourceTexture,
                sourceTexture->getDesc().format,
                sourceSubresources,
                sourceDimension)
        };
    }

    auto sourceBindingSet =
        resource_allocator.create(binding_set_desc, binding_layout.Get());
    MARK_DESTROY_NVRHI_RESOURCE(sourceBindingSet);

    nvrhi::GraphicsState state;
    state.pipeline = pso;
    state.framebuffer = targetFramebuffer;
    state.bindings = { sourceBindingSet };
    state.viewport.addViewport(targetViewport);
    state.viewport.addScissorRect(nvrhi::Rect(targetViewport));

    BlitConstants blitConstants = {};
    blitConstants.sourceOrigin = blit_parameters.sourceBox.GetMin();
    blitConstants.sourceSize = blit_parameters.sourceBox.GetSize();
    blitConstants.targetOrigin = blit_parameters.targetBox.GetMin();
    blitConstants.targetSize = blit_parameters.targetBox.GetSize();

    commandList->open();

    commandList->writeBuffer(
        constant_buffer.Get(), &blitConstants, sizeof(blitConstants));
    commandList->setGraphicsState(state);

    nvrhi::DrawArguments args;
    args.instanceCount = 1;
    args.vertexCount = 4;
    commandList->draw(args);

    commandList->close();
    resource_allocator.device->executeCommandList(commandList);

    params.set_output("Tex", output);
    return true;
}

NODE_DECLARATION_UI(blit_to_present);
NODE_DEF_CLOSE_SCOPE
