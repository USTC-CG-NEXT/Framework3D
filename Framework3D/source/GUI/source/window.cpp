#include "GUI/window.h"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <imgui.h>

#include <RHI/rhi.hpp>
#include <format>

#include "RHI/DeviceManager/DeviceManager.h"
#include "RHI/ShaderFactory/shader.hpp"
#include "imgui_renderer.h"
#include "vulkan/vulkan.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

class DockingImguiRenderer final : public ImGui_Renderer {
    friend class Window;

   public:
    explicit DockingImguiRenderer(DeviceManager* devManager)
        : ImGui_Renderer(devManager)
    {
    }

    bool JoystickButtonUpdate(int button, bool pressed) override;
    bool JoystickAxisUpdate(int axis, float value) override;
    bool KeyboardUpdate(int key, int scancode, int action, int mods) override;
    bool KeyboardCharInput(unsigned unicode, int mods) override;
    bool MousePosUpdate(double xpos, double ypos) override;
    bool MouseScrollUpdate(double xoffset, double yoffset) override;
    bool MouseButtonUpdate(int button, int action, int mods) override;
    void Animate(float elapsedTimeSeconds) override;

   private:
    void register_widget(std::unique_ptr<IWidget> widget);
    void buildUI() override;

    std::vector<std::unique_ptr<IWidget>> widgets_;
};
bool DockingImguiRenderer::JoystickButtonUpdate(int button, bool pressed)
{
    for (auto&& widget : widgets_) {
        if (widget->JoystickButtonUpdate(button, pressed)) {
            return true;
        }
    }
    return ImGui_Renderer::JoystickButtonUpdate(button, pressed);
}

bool DockingImguiRenderer::JoystickAxisUpdate(int axis, float value)
{
    for (auto&& widget : widgets_) {
        if (widget->JoystickAxisUpdate(axis, value)) {
            return true;
        }
    }
    return ImGui_Renderer::JoystickAxisUpdate(axis, value);
}

bool DockingImguiRenderer::KeyboardUpdate(
    int key,
    int scancode,
    int action,
    int mods)
{
    for (auto&& widget : widgets_) {
        if (widget->KeyboardUpdate(key, scancode, action, mods)) {
            return true;
        }
    }
    return ImGui_Renderer::KeyboardUpdate(key, scancode, action, mods);
}

bool DockingImguiRenderer::KeyboardCharInput(unsigned unicode, int mods)
{
    for (auto&& widget : widgets_) {
        if (widget->KeyboardCharInput(unicode, mods)) {
            return true;
        }
    }
    return ImGui_Renderer::KeyboardCharInput(unicode, mods);
}

bool DockingImguiRenderer::MousePosUpdate(double xpos, double ypos)
{
    for (auto&& widget : widgets_) {
        if (widget->MousePosUpdate(xpos, ypos)) {
            return true;
        }
    }
    return ImGui_Renderer::MousePosUpdate(xpos, ypos);
}

bool DockingImguiRenderer::MouseScrollUpdate(double xoffset, double yoffset)
{
    for (auto&& widget : widgets_) {
        if (widget->MouseScrollUpdate(xoffset, yoffset)) {
            return true;
        }
    }
    return ImGui_Renderer::MouseScrollUpdate(xoffset, yoffset);
}

bool DockingImguiRenderer::MouseButtonUpdate(int button, int action, int mods)
{
    for (auto&& widget : widgets_) {
        if (widget->MouseButtonUpdate(button, action, mods)) {
            return true;
        }
    }
    return ImGui_Renderer::MouseButtonUpdate(button, action, mods);
}

void DockingImguiRenderer::Animate(float elapsedTimeSeconds)
{
    for (auto&& widget : widgets_) {
        widget->Animate(elapsedTimeSeconds);
    }
    ImGui_Renderer::Animate(elapsedTimeSeconds);
}

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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (widget->Begin()) {
            ImGui::PopStyleVar(1);
            widget->SetStatus();

            if (widget->SizeChanged()) {
                widget->BackBufferResized(widget->Width(), widget->Height(), 1);
            }

            widget->BuildUI();

            widget->End();
        }
        else {
            ImGui::PopStyleVar(1);
        }
        widget->AlwaysEnd();
        if (!widget->IsOpen()) {
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
    RHI::init(true);

    auto manager = RHI::internal::get_device_manager();
    imguiRenderPass = std::make_unique<DockingImguiRenderer>(manager);
    imguiRenderPass->Init(std::make_shared<ShaderFactory>());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    manager->AddRenderPassToBack(imguiRenderPass.get());
}

Window::~Window()
{
    auto manager = RHI::internal::get_device_manager();

    manager->RemoveRenderPass(imguiRenderPass.get());
    imguiRenderPass.reset();
    RHI::shutdown();
}

void Window::run()
{
    auto manager = RHI::internal::get_device_manager();
    manager->RunMessageLoop();
}

void Window::register_widget(std::unique_ptr<IWidget> unique)
{
    unique->SetWindow(this);
    imguiRenderPass->register_widget(std::move(unique));
}

USTC_CG_NAMESPACE_CLOSE_SCOPE