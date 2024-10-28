#include "GUI/window.h"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <RHI/rhi.hpp>
#include <format>
#include <stdexcept>
#include <unordered_map>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Logging/Logging.h"
#include "RHI/DeviceManager/DeviceManager.h"
#include "RHI/ShaderFactory/shader.hpp"
#include "RHI/internal/resources.hpp"
#include "imgui_renderer.h"
#include "vulkan/vulkan.hpp"

namespace USTC_CG {

class DockingImguiRenderer final : public ImGui_Renderer {
    friend class Window;

   public:
    explicit DockingImguiRenderer(DeviceManager* devManager)
        : ImGui_Renderer(devManager)
    {
    }

   protected:
    void register_dock();
    void buildUI() override;
};

void DockingImguiRenderer::register_dock()
{
}

void DockingImguiRenderer::buildUI()
{
}

Window::Window()
{
    rhi::init(true);

    auto manager = rhi::internal::get_device_manager();
    imguiRenderPass = std::make_unique<DockingImguiRenderer>(manager);
    imguiRenderPass->Init(std::make_shared<ShaderFactory>());

    manager->AddRenderPassToBack(imguiRenderPass.get());
}

Window::~Window()
{
    rhi::shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    // glfwDestroyWindow(window_);
    // glfwTerminate();
}

void Window::run()
{
    auto manager = rhi::internal::get_device_manager();
    manager->RunMessageLoop();
    // glfwShowWindow(window_);

    // while (!glfwWindowShouldClose(window_)) {
    //     if (!glfwGetWindowAttrib(window_, GLFW_VISIBLE) ||
    //         glfwGetWindowAttrib(window_, GLFW_ICONIFIED))
    //         glfwWaitEvents();
    //     else {
    //         glfwPollEvents();
    //         render();
    //     }
    // }
}

void Window::BuildUI()
{
    ImGui::ShowDemoWindow();
}

void Window::Render()
{
    log::info("Empty render function for window.");
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
    // glfwMakeContextCurrent(window_);
    // glfwSwapInterval(1);  // Enable vsync

    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();

    // ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //// io.ConfigFlags |=
    ////     ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport /
    // Platform
    //     //  - fontsize
    //     float xscale,
    //     yscale;
    // glfwGetWindowContentScale(window_, &xscale, &yscale);
    //// - style
    // ImGui::StyleColorsDark();

    // io.DisplayFramebufferScale.x = xscale;
    // io.DisplayFramebufferScale.x = yscale;

    // ImGui_ImplGlfw_InitForVulkan(window_, true);

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
    // vkCmdBeginRenderPass(command_buffer, &info,
    // VK_SUBPASS_CONTENTS_INLINE);
    // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    // vkCmdEndRenderPass(command_buffer);
    // end_command_buffer(command_buffer);
    // submit_command_buffer(command_buffer);

    glfwSwapBuffers(window_);
}
}  // namespace USTC_CG
