#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "GUI/window/window.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <RHI/rhi.hpp>
#include <format>
#include <stdexcept>
#include <unordered_map>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Logging/Logging.h"
#include "RHI/shader.hpp"
#include "vulkan/vulkan.hpp"

namespace USTC_CG {

struct ImGui_NVRHI {
    ImGui_NVRHI();
    nvrhi::DeviceHandle renderer;
    nvrhi::CommandListHandle m_commandList;

    nvrhi::ShaderHandle vertexShader;
    nvrhi::ShaderHandle pixelShader;
    nvrhi::InputLayoutHandle shaderAttribLayout;

    nvrhi::TextureHandle fontTexture;
    nvrhi::SamplerHandle fontSampler;

    nvrhi::BufferHandle vertexBuffer;
    nvrhi::BufferHandle indexBuffer;

    nvrhi::BindingLayoutHandle bindingLayout;
    nvrhi::GraphicsPipelineDesc basePSODesc;

    nvrhi::GraphicsPipelineHandle pso;
    std::unordered_map<nvrhi::ITexture*, nvrhi::BindingSetHandle> bindingsCache;

    std::vector<ImDrawVert> vtxBuffer;
    std::vector<ImDrawIdx> idxBuffer;

    bool init(nvrhi::DeviceHandle renderer);
    bool beginFrame(float elapsedTimeSeconds);
    bool render(nvrhi::IFramebuffer* framebuffer);
    void backbufferResizing();

   private:
    bool reallocateBuffer(
        nvrhi::BufferHandle& buffer,
        size_t requiredSize,
        size_t reallocateSize,
        bool isIndexBuffer);

    bool createFontTexture(nvrhi::ICommandList* commandList);

    nvrhi::IGraphicsPipeline* getPSO(nvrhi::IFramebuffer* fb);
    nvrhi::IBindingSet* getBindingSet(nvrhi::ITexture* texture);
    bool updateGeometry(nvrhi::ICommandList* commandList);

    std::unique_ptr<ShaderFactory> shader_factory;
};

struct VERTEX_CONSTANT_BUFFER {
    float mvp[4][4];
};

bool ImGui_NVRHI::createFontTexture(nvrhi::ICommandList* commandList)
{
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    {
        nvrhi::TextureDesc desc;
        desc.width = width;
        desc.height = height;
        desc.format = nvrhi::Format::RGBA8_UNORM;
        desc.debugName = "ImGui font texture";

        fontTexture = renderer->createTexture(desc);

        commandList->beginTrackingTextureState(
            fontTexture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common);

        if (fontTexture == nullptr)
            return false;

        commandList->writeTexture(fontTexture, 0, 0, pixels, width * 4);

        commandList->setPermanentTextureState(
            fontTexture, nvrhi::ResourceStates::ShaderResource);
        commandList->commitBarriers();

        io.Fonts->TexID = fontTexture;
    }

    {
        const auto desc =
            nvrhi::SamplerDesc()
                .setAllAddressModes(nvrhi::SamplerAddressMode::Wrap)
                .setAllFilters(true);

        fontSampler = renderer->createSampler(desc);

        if (fontSampler == nullptr)
            return false;
    }

    return true;
}

ImGui_NVRHI::ImGui_NVRHI()
{
    shader_factory = std::make_unique<ShaderFactory>();
}

bool ImGui_NVRHI::init(nvrhi::DeviceHandle renderer)
{
    this->renderer = renderer;

    m_commandList = renderer->createCommandList();

    m_commandList->open();

    std::string error_string;
    nvrhi::BindingLayoutDescVector binding_layout;

    vertexShader = shader_factory->compile_shader(
        "main",
        nvrhi::ShaderType::Vertex,
        "imgui_shader/imgui_vertex.slang",
        binding_layout,
        error_string,
        {});

    pixelShader = shader_factory->compile_shader(
        "main",
        nvrhi::ShaderType::Pixel,
        "imgui_shader/imgui_vertex.slang",
        binding_layout,
        error_string,
        {});

    if (!vertexShader || !pixelShader) {
        logging("Failed to create an ImGUI shader");
        return false;
    }

    // create attribute layout object
    nvrhi::VertexAttributeDesc vertexAttribLayout[] = {
        { "POSITION",
          nvrhi::Format::RG32_FLOAT,
          1,
          0,
          offsetof(ImDrawVert, pos),
          sizeof(ImDrawVert),
          false },
        { "TEXCOORD",
          nvrhi::Format::RG32_FLOAT,
          1,
          0,
          offsetof(ImDrawVert, uv),
          sizeof(ImDrawVert),
          false },
        { "COLOR",
          nvrhi::Format::RGBA8_UNORM,
          1,
          0,
          offsetof(ImDrawVert, col),
          sizeof(ImDrawVert),
          false },
    };

    shaderAttribLayout = renderer->createInputLayout(
        vertexAttribLayout,
        sizeof(vertexAttribLayout) / sizeof(vertexAttribLayout[0]),
        vertexShader);

    // add the default font - before creating the font texture
    auto& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    // create font texture
    if (!createFontTexture(m_commandList)) {
        return false;
    }

    // create PSO
    {
        nvrhi::BlendState blendState;
        blendState.targets[0]
            .setBlendEnable(true)
            .setSrcBlend(nvrhi::BlendFactor::SrcAlpha)
            .setDestBlend(nvrhi::BlendFactor::InvSrcAlpha)
            .setSrcBlendAlpha(nvrhi::BlendFactor::InvSrcAlpha)
            .setDestBlendAlpha(nvrhi::BlendFactor::Zero);

        auto rasterState = nvrhi::RasterState()
                               .setFillSolid()
                               .setCullNone()
                               .setScissorEnable(true)
                               .setDepthClipEnable(true);

        auto depthStencilState =
            nvrhi::DepthStencilState()
                .disableDepthTest()
                .enableDepthWrite()
                .disableStencil()
                .setDepthFunc(nvrhi::ComparisonFunc::Always);

        nvrhi::RenderState renderState;
        renderState.blendState = blendState;
        renderState.depthStencilState = depthStencilState;
        renderState.rasterState = rasterState;

        nvrhi::BindingLayoutDesc layoutDesc;
        layoutDesc.visibility = nvrhi::ShaderType::All;
        layoutDesc.bindings = { nvrhi::BindingLayoutItem::PushConstants(
                                    0, sizeof(float) * 2),
                                nvrhi::BindingLayoutItem::Texture_SRV(0),
                                nvrhi::BindingLayoutItem::Sampler(0) };
        bindingLayout = renderer->createBindingLayout(layoutDesc);

        basePSODesc.primType = nvrhi::PrimitiveType::TriangleList;
        basePSODesc.inputLayout = shaderAttribLayout;
        basePSODesc.VS = vertexShader;
        basePSODesc.PS = pixelShader;
        basePSODesc.renderState = renderState;
        basePSODesc.bindingLayouts = { bindingLayout };
    }

    m_commandList->close();
    renderer->executeCommandList(m_commandList);

    return true;
}

bool ImGui_NVRHI::reallocateBuffer(
    nvrhi::BufferHandle& buffer,
    size_t requiredSize,
    size_t reallocateSize,
    const bool indexBuffer)
{
    if (buffer == nullptr ||
        size_t(buffer->getDesc().byteSize) < requiredSize) {
        nvrhi::BufferDesc desc;
        desc.byteSize = uint32_t(reallocateSize);
        desc.structStride = 0;
        desc.debugName =
            indexBuffer ? "ImGui index buffer" : "ImGui vertex buffer";
        desc.canHaveUAVs = false;
        desc.isVertexBuffer = !indexBuffer;
        desc.isIndexBuffer = indexBuffer;
        desc.isDrawIndirectArgs = false;
        desc.isVolatile = false;
        desc.initialState = indexBuffer ? nvrhi::ResourceStates::IndexBuffer
                                        : nvrhi::ResourceStates::VertexBuffer;
        desc.keepInitialState = true;

        buffer = renderer->createBuffer(desc);

        if (!buffer) {
            return false;
        }
    }

    return true;
}

bool ImGui_NVRHI::beginFrame(float elapsedTimeSeconds)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = elapsedTimeSeconds;
    io.MouseDrawCursor = false;

    ImGui::NewFrame();

    return true;
}

nvrhi::IGraphicsPipeline* ImGui_NVRHI::getPSO(nvrhi::IFramebuffer* fb)
{
    if (pso)
        return pso;

    pso = renderer->createGraphicsPipeline(basePSODesc, fb);
    assert(pso);

    return pso;
}

nvrhi::IBindingSet* ImGui_NVRHI::getBindingSet(nvrhi::ITexture* texture)
{
    auto iter = bindingsCache.find(texture);
    if (iter != bindingsCache.end()) {
        return iter->second;
    }

    nvrhi::BindingSetDesc desc;

    desc.bindings = { nvrhi::BindingSetItem::PushConstants(
                          0, sizeof(float) * 2),
                      nvrhi::BindingSetItem::Texture_SRV(0, texture),
                      nvrhi::BindingSetItem::Sampler(0, fontSampler) };

    nvrhi::BindingSetHandle binding;
    binding = renderer->createBindingSet(desc, bindingLayout);
    assert(binding);

    bindingsCache[texture] = binding;
    return binding;
}

bool ImGui_NVRHI::updateGeometry(nvrhi::ICommandList* commandList)
{
    ImDrawData* drawData = ImGui::GetDrawData();

    // create/resize vertex and index buffers if needed
    if (!reallocateBuffer(
            vertexBuffer,
            drawData->TotalVtxCount * sizeof(ImDrawVert),
            (drawData->TotalVtxCount + 5000) * sizeof(ImDrawVert),
            false)) {
        return false;
    }

    if (!reallocateBuffer(
            indexBuffer,
            drawData->TotalIdxCount * sizeof(ImDrawIdx),
            (drawData->TotalIdxCount + 5000) * sizeof(ImDrawIdx),
            true)) {
        return false;
    }

    vtxBuffer.resize(vertexBuffer->getDesc().byteSize / sizeof(ImDrawVert));
    idxBuffer.resize(indexBuffer->getDesc().byteSize / sizeof(ImDrawIdx));

    // copy and convert all vertices into a single contiguous buffer
    ImDrawVert* vtxDst = &vtxBuffer[0];
    ImDrawIdx* idxDst = &idxBuffer[0];

    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList* cmdList = drawData->CmdLists[n];

        memcpy(
            vtxDst,
            cmdList->VtxBuffer.Data,
            cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(
            idxDst,
            cmdList->IdxBuffer.Data,
            cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

        vtxDst += cmdList->VtxBuffer.Size;
        idxDst += cmdList->IdxBuffer.Size;
    }

    commandList->writeBuffer(
        vertexBuffer, &vtxBuffer[0], vertexBuffer->getDesc().byteSize);
    commandList->writeBuffer(
        indexBuffer, &idxBuffer[0], indexBuffer->getDesc().byteSize);

    return true;
}

bool ImGui_NVRHI::render(nvrhi::IFramebuffer* framebuffer)
{
    ImDrawData* drawData = ImGui::GetDrawData();
    const auto& io = ImGui::GetIO();

    m_commandList->open();
    m_commandList->beginMarker("ImGUI");

    if (!updateGeometry(m_commandList)) {
        return false;
    }

    // handle DPI scaling
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    float invDisplaySize[2] = { 1.f / io.DisplaySize.x,
                                1.f / io.DisplaySize.y };

    // set up graphics state
    nvrhi::GraphicsState drawState;

    drawState.framebuffer = framebuffer;
    assert(drawState.framebuffer);

    drawState.pipeline = getPSO(drawState.framebuffer);

    drawState.viewport.viewports.push_back(nvrhi::Viewport(
        io.DisplaySize.x * io.DisplayFramebufferScale.x,
        io.DisplaySize.y * io.DisplayFramebufferScale.y));
    drawState.viewport.scissorRects.resize(1);  // updated below

    nvrhi::VertexBufferBinding vbufBinding;
    vbufBinding.buffer = vertexBuffer;
    vbufBinding.slot = 0;
    vbufBinding.offset = 0;
    drawState.vertexBuffers.push_back(vbufBinding);

    drawState.indexBuffer.buffer = indexBuffer;
    drawState.indexBuffer.format =
        (sizeof(ImDrawIdx) == 2 ? nvrhi::Format::R16_UINT
                                : nvrhi::Format::R32_UINT);
    drawState.indexBuffer.offset = 0;

    // render command lists
    int vtxOffset = 0;
    int idxOffset = 0;
    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        for (int i = 0; i < cmdList->CmdBuffer.Size; i++) {
            const ImDrawCmd* pCmd = &cmdList->CmdBuffer[i];

            if (pCmd->UserCallback) {
                pCmd->UserCallback(cmdList, pCmd);
            }
            else {
                drawState.bindings = { getBindingSet(
                    (nvrhi::ITexture*)pCmd->TextureId) };
                assert(drawState.bindings[0]);

                drawState.viewport.scissorRects[0] = nvrhi::Rect(
                    int(pCmd->ClipRect.x),
                    int(pCmd->ClipRect.z),
                    int(pCmd->ClipRect.y),
                    int(pCmd->ClipRect.w));

                nvrhi::DrawArguments drawArguments;
                drawArguments.vertexCount = pCmd->ElemCount;
                drawArguments.startIndexLocation = idxOffset;
                drawArguments.startVertexLocation = vtxOffset;

                m_commandList->setGraphicsState(drawState);
                m_commandList->setPushConstants(
                    invDisplaySize, sizeof(invDisplaySize));
                m_commandList->drawIndexed(drawArguments);
            }

            idxOffset += pCmd->ElemCount;
        }

        vtxOffset += cmdList->VtxBuffer.Size;
    }

    m_commandList->endMarker();
    m_commandList->close();
    renderer->executeCommandList(m_commandList);

    return true;
}

void ImGui_NVRHI::backbufferResizing()
{
    pso = nullptr;
}

Window::Window(const std::string& window_name) : name_(window_name)
{
    if (!init_glfw()) {
        // Initialize GLFW and check for failure
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    window_ =
        glfwCreateWindow(width_, height_, name_.c_str(), nullptr, nullptr);
    if (window_ == nullptr) {
        glfwTerminate();  // Ensure GLFW is cleaned up before throwing
        throw std::runtime_error("Failed to create GLFW window!");
    }

    if (!init_gui()) {
        // Initialize the GUI and check for failure
        glfwDestroyWindow(window_);
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GUI!");
    }

    imgui_nvrhi = std::make_unique<ImGui_NVRHI>();
    imgui_nvrhi->init(get_device());
}

Window::~Window()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();
}

void Window::run()
{
    glfwShowWindow(window_);

    while (!glfwWindowShouldClose(window_)) {
        if (!glfwGetWindowAttrib(window_, GLFW_VISIBLE) ||
            glfwGetWindowAttrib(window_, GLFW_ICONIFIED))
            glfwWaitEvents();
        else {
            glfwPollEvents();
            render();
        }
    }
}

void Window::BuildUI()
{
    ImGui::ShowDemoWindow();
}

void Window::Render()
{
    USTC_CG::logging("Empty render function for window.", Info);
}

bool Window::init_glfw()
{
    glfwSetErrorCallback([](int error, const char* desc) {
        fprintf(stderr, "GLFW Error %d: %s\n", error, desc);
    });
    if (!glfwInit()) {
        return false;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

    return true;
}

bool Window::init_gui()
{
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);  // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |=
    //     ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform
    //  - fontsize
    float xscale, yscale;
    glfwGetWindowContentScale(window_, &xscale, &yscale);
    // - style
    ImGui::StyleColorsDark();

    io.DisplayFramebufferScale.x = xscale;
    io.DisplayFramebufferScale.x = yscale;

    ImGui_ImplGlfw_InitForVulkan(window_, true);

    return true;
}

void Window::render()
{
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoNavFocus |
                    ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin(("DockSpace" + std::to_string(0)).c_str(), 0, window_flags);
    ImGui::PopStyleVar(3);
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

    ImGui::DockSpace(
        dockspace_id,
        ImVec2(0.0f, 0.0f),
        ImGuiDockNodeFlags_PassthruCentralNode);

    Render();
    BuildUI();

    ImGui::End();

    ImGui::Render();

    

    // Record Vulkan command buffer and submit it to the queue
    // Example:
    // VkCommandBuffer command_buffer = begin_command_buffer();
    // VkClearValue clear_value = {};
    // clear_value.color = {0.35f, 0.45f, 0.50f, 1.00f};
    // VkRenderPassBeginInfo info = {};
    // info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // info.renderPass = render_pass;
    // info.framebuffer = framebuffer;
    // info.renderArea.extent.width = width_;
    // info.renderArea.extent.height = height_;
    // info.clearValueCount = 1;
    // info.pClearValues = &clear_value;
    // vkCmdBeginRenderPass(command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    // vkCmdEndRenderPass(command_buffer);
    // end_command_buffer(command_buffer);
    // submit_command_buffer(command_buffer);

    glfwSwapBuffers(window_);
}
}  // namespace USTC_CG
