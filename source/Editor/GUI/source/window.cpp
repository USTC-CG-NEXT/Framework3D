#include "GUI/window.h"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <imgui.h>

#include <RHI/rhi.hpp>
#include <format>

#include "GUI/ImGuiFileDialog.h"
#include "RHI/DeviceManager/DeviceManager.h"
#include "RHI/ShaderFactory/shader.hpp"
#include "imgui_renderer.h"
#include "vulkan/vulkan.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

class DockingImguiRenderer final : public ImGui_Renderer {
    friend class Window;

   public:
    explicit DockingImguiRenderer(Window* window, DeviceManager* devManager)
        : window_(window),
          ImGui_Renderer(devManager)
    {
    }

    ~DockingImguiRenderer() override;

    bool JoystickButtonUpdate(int button, bool pressed) override;
    bool JoystickAxisUpdate(int axis, float value) override;
    bool KeyboardUpdate(int key, int scancode, int action, int mods) override;
    bool KeyboardCharInput(unsigned unicode, int mods) override;
    bool MousePosUpdate(double xpos, double ypos) override;
    bool MouseScrollUpdate(double xoffset, double yoffset) override;
    bool MouseButtonUpdate(int button, int action, int mods) override;
    void Animate(float elapsedTimeSeconds) override;
    void register_function_before_frame(
        const std::function<void(Window*)>& callback);
    void register_function_after_frame(
        const std::function<void(Window*)>& callback);
    void register_openable_widget(
        std::unique_ptr<IWidgetFactory>& widget_factory,
        const std::vector<std::string>& menu_item);

   private:
    void register_widget(std::unique_ptr<IWidget> widget);
    void drawMenuBar();
    void buildUI() override;

    std::vector<std::unique_ptr<IWidget>> widgets_;
    Window* window_;
    std::vector<std::function<void(Window*)>> callbacks_before_frame_;
    std::vector<std::function<void(Window*)>> callbacks_after_frame_;

    struct MenuNode {
        std::unordered_map<std::string, std::unique_ptr<MenuNode>> children;
        std::unique_ptr<IWidgetFactory> widget_factory;

        void register_node(
            const std::vector<std::string>& path,
            std::unique_ptr<IWidgetFactory>& factory)
        {
            if (path.empty()) {
                widget_factory = std::move(factory);
                return;
            }

            auto& child = children[path.front()];
            if (!child) {
                child = std::make_unique<MenuNode>();
            }

            child->register_node(
                std::vector(path.begin() + 1, path.end()), factory);
        }
    };

    MenuNode menu_tree;

    void recursive_draw(MenuNode& node);
};

DockingImguiRenderer::~DockingImguiRenderer()
{
    callbacks_after_frame_.clear();

    // widgets_ should be cleared from the last to the first instead of using
    // widgets_.clear();

    for (auto it = widgets_.rbegin(); it != widgets_.rend(); ++it) {
        it->reset();
    }
}

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
    window_->elapsedTimeSeconds = elapsedTimeSeconds;
}

void DockingImguiRenderer::register_function_before_frame(
    const std::function<void(Window*)>& callback)
{
    callbacks_before_frame_.push_back(callback);
}

void DockingImguiRenderer::register_function_after_frame(
    const std::function<void(Window*)>& callback)
{
    callbacks_after_frame_.push_back(callback);
}

void DockingImguiRenderer::register_openable_widget(
    std::unique_ptr<IWidgetFactory>& widget_factory,
    const std::vector<std::string>& menu_item)
{
    menu_tree.register_node(menu_item, widget_factory);
}

void DockingImguiRenderer::register_widget(std::unique_ptr<IWidget> widget)
{
    // If the widget with the "UniqueName" exists, replace it
    std::string unique_name = widget->GetWindowUniqueName();
    for (auto& w : widgets_) {
        if (w->GetWindowUniqueName() == unique_name) {
            w = std::move(widget);
            return;
        }
    }

    widgets_.push_back(std::move(widget));
}

void DockingImguiRenderer::drawMenuBar()
{
    if (ImGui::BeginMenuBar()) {
        recursive_draw(menu_tree);
        ImGui::EndMenuBar();
    }
}

void DockingImguiRenderer::buildUI()
{
    for (auto&& callback : callbacks_before_frame_) {
        callback(window_);
    }
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
    drawMenuBar();

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

            if (widget->SizeChanged()) {
                widget->BackBufferResized(widget->Width(), widget->Height(), 1);
            }

            widget->BuildUI();
            widget->SetStatus();
        }
        else {
            ImGui::PopStyleVar(1);
        }

        widget->End();

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
    for (size_t i = 0; i < widgets_.size(); ++i) {
        widgets_[i]->CallBack();
    }
    ImGui::End();

    for (auto&& callback : callbacks_after_frame_) {
        callback(window_);
    }
}

void DockingImguiRenderer::recursive_draw(MenuNode& node)
{
    for (auto& [name, child] : node.children) {
        if (child->children.empty()) {
            if (ImGui::MenuItem(name.c_str())) {
                auto widget = child->widget_factory->Create(widgets_);
                register_widget(std::move(widget));
            }
        }
        else {
            if (ImGui::BeginMenu(name.c_str())) {
                recursive_draw(*child);
                ImGui::EndMenu();
            }
        }
    }
}

Window::Window()
{
    RHI::init(true);

    auto manager = RHI::internal::get_device_manager();
    imguiRenderPass = std::make_unique<DockingImguiRenderer>(this, manager);
    imguiRenderPass->Init(std::make_shared<ShaderFactory>());

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    auto instance = IGFD::FileDialog::Instance();
    IGFD::FileDialogConfig config;
    config.path = "../../Assets";
    instance->OpenDialog(
        "SelectFile", "Select File", "{ .usd, .usda, .usdz }", config);

    manager->AddRenderPassToBack(imguiRenderPass.get());
}

Window::~Window()
{
    auto manager = RHI::internal::get_device_manager();

    manager->RemoveRenderPass(imguiRenderPass.get());
    imguiRenderPass.reset();
    RHI::shutdown();
}

float Window::get_elapsed_time()
{
    return elapsedTimeSeconds;
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

void Window::register_function_before_frame(
    const std::function<void(Window*)>& callback)
{
    imguiRenderPass->register_function_before_frame(callback);
}

void Window::register_function_after_frame(
    const std::function<void(Window*)>& callback)
{
    imguiRenderPass->register_function_after_frame(callback);
}

void Window::register_openable_widget(
    std::unique_ptr<IWidgetFactory> window_factory,
    const std::vector<std::string>& menu_item)
{
    imguiRenderPass->register_openable_widget(window_factory, menu_item);
}
IWidget* Window::get_widget(const std::string& unique_name) const
{
    for (auto& widget : imguiRenderPass->widgets_) {
        if (widget->GetWindowUniqueName() == unique_name) {
            return widget.get();
        }
    }
}

std::vector<IWidget*> Window::get_widgets() const
{
    std::vector<IWidget*> widgets;
    for (auto& widget : imguiRenderPass->widgets_) {
        widgets.push_back(widget.get());
    }
    return widgets;
}

void Window::set_all_node_system_dirty()
{
    for (auto& widget : imguiRenderPass->widgets_) {
        if (std::strcmp(widget->GetWindowName(), "Node editor") == 0) {
            widget->SetNodeSystemDirty(true);
        }
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE