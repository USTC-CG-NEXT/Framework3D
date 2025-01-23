#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

#include "GUI/api.h"
#include "widget.h"
struct GLFWwindow;

USTC_CG_NAMESPACE_OPEN_SCOPE

class DockingImguiRenderer;

// Represents a window in a GUI application, providing basic functionalities
// such as initialization and rendering.
class GUI_API Window {
   public:
    // Constructor that sets the window's title.
    explicit Window();

    virtual ~Window();

    // Enters the main rendering loop.
    void run();
    void register_widget(std::unique_ptr<IWidget> unique);
    void register_function_perframe(
        const std::function<void(Window *)> &callback);

    void register_openable_widget(
        std::unique_ptr<IWidgetFactory> window_factory,
        const std::vector<std::string>& menu_item);

   protected:
    std::unique_ptr<DockingImguiRenderer> imguiRenderPass;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE