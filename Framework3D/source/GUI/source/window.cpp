#include "GUI/window.h"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <imgui.h>

#include <RHI/rhi.hpp>
#include <format>

#include "RHI/DeviceManager/DeviceManager.h"
#include "RHI/ShaderFactory/shader.hpp"
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

   private:
    void register_widget(std::unique_ptr<IWidget> widget);
    void buildUI() override;

    std::vector<std::unique_ptr<IWidget>> widgets_;
};

void DockingImguiRenderer::register_widget(std::unique_ptr<IWidget> widget)
{
    widgets_.push_back(std::move(widget));
}

void DockingImguiRenderer::buildUI()
{
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

    std::vector<IWidget*> widget_to_remove;
    for (auto& widget : widgets_) {
        if (!widget->BuildUI()) {
            widget_to_remove.push_back(widget.get());
        }
    }

    for (auto widget : widget_to_remove) {
        widgets_.erase(
            std::remove_if(
                widgets_.begin(),
                widgets_.end(),
                [widget](const std::unique_ptr<IWidget>& w) {
                    return w.get() == widget;
                }),
            widgets_.end());
    }

    ImGui::End();
}

Window::Window()
{
    rhi::init(true);

    auto manager = rhi::internal::get_device_manager();
    imguiRenderPass = std::make_unique<DockingImguiRenderer>(manager);
    imguiRenderPass->Init(std::make_shared<ShaderFactory>());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    manager->AddRenderPassToBack(imguiRenderPass.get());
}

Window::~Window()
{
    auto manager = rhi::internal::get_device_manager();

    manager->RemoveRenderPass(imguiRenderPass.get());
    imguiRenderPass.reset();
    rhi::shutdown();
}

void Window::run()
{
    auto manager = rhi::internal::get_device_manager();
    manager->RunMessageLoop();
}

void Window::register_widget(std::unique_ptr<IWidget> unique)
{
    unique->set_window(this);
    imguiRenderPass->register_widget(std::move(unique));
}

USTC_CG_NAMESPACE_CLOSE_SCOPE