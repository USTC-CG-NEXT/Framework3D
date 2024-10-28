#pragma once

#include <memory>
#include <string>

#include "USTC_CG.h"
#include "widget.h"
struct GLFWwindow;

USTC_CG_NAMESPACE_OPEN_SCOPE

class DockingImguiRenderer;

// Represents a window in a GUI application, providing basic functionalities
// such as initialization and rendering.
class Window {
   public:
    // Constructor that sets the window's title.
    explicit Window();

    virtual ~Window();

    // Enters the main rendering loop.
    void run();
    void register_widget(std::unique_ptr<IWidget> unique);

   protected:
    std::unique_ptr<DockingImguiRenderer> imguiRenderPass;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE