#include "GUI/window/window.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <format>
#include <stdexcept>

#include "GLFW/glfw3.h"
#include "Logging/Logging.h"
#include "imgui_internal.h"

namespace USTC_CG {
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
}

Window::~Window()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool Window::init()
{
    // Placeholder for additional initialization if needed.
    return true;
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

    // Initialize Vulkan here and pass the Vulkan device, physical device, etc. to ImGui_ImplVulkan_Init
    // Example:
    // ImGui_ImplVulkan_InitInfo init_info = {};
    // init_info.Instance = instance;
    // init_info.PhysicalDevice = physical_device;
    // init_info.Device = device;
    // init_info.QueueFamily = queue_family;
    // init_info.Queue = queue;
    // init_info.PipelineCache = pipeline_cache;
    // init_info.DescriptorPool = descriptor_pool;
    // init_info.Allocator = allocator;
    // init_info.MinImageCount = min_image_count;
    // init_info.ImageCount = image_count;
    // init_info.CheckVkResultFn = check_vk_result;
    // ImGui_ImplVulkan_Init(&init_info, render_pass);

    return true;
}

void Window::render()
{
    ImGui_ImplVulkan_NewFrame();
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
