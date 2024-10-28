#pragma once

#include <memory>
#include <string>

#include "USTC_CG.h"
//#include "nvrhi/nvrhi.h"
//struct GLFWwindow;
//
//namespace USTC_CG {
//
//struct ImGui_NVRHI;
//
//// Represents a window in a GUI application, providing basic functionalities
//// such as initialization and rendering.
//class Window {
//   public:
//    // Constructor that sets the window's title.
//    explicit Window(const std::string& window_name);
//
//    virtual ~Window();
//
//    // Enters the main rendering loop.
//    void run();
//
//   protected:
//    // Virtual draw function to be implemented by derived classes for custom
//    // rendering.
//    virtual void BuildUI();
//
//    virtual void Render();
//
//    // Initializes GLFW library.
//    bool init_glfw();
//
//    // Initializes the GUI library (e.g., Dear ImGui).
//    bool init_gui();
//
//    // Handles the rendering of each frame.
//    void render();
//
//    std::string name_;              // Name (title) of the window.
//    GLFWwindow* window_ = nullptr;  // Pointer to the GLFW window.
//    int width_ = 1920;              // Width of the window.
//    int height_ = 1080;             // Height of the window.
//
//    std::unique_ptr<ImGui_NVRHI> imgui_nvrhi;  // ImGui NVRHI instance.
//};
//}  // namespace USTC_CG
