#pragma once

#include <memory>

#include "GUI/widget.h"
#include "polyscope/camera_parameters.h"
#include "polyscope/polyscope.h"
#include "polyscope/types.h"
#include "polyscope/view.h"
#include "polyscope_widget/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class BaseCamera;
class FreeCamera;
class NodeTree;

extern polyscope::CameraParameters default_camera_params;
struct PolyscopeRenderPrivateData;

class POLYSCOPE_WIDGET_API PolyscopeRenderer final : public IWidget {
   public:
    explicit PolyscopeRenderer(
        polyscope::CameraParameters prams = default_camera_params);
    ~PolyscopeRenderer() override;

    bool BuildUI() override;
    // void SetCallBack(const std::function<void(Window*, IWidget*)>&) override;

   protected:
    ImGuiWindowFlags GetWindowFlag() override;
    const char* GetWindowName() override;
    std::string GetWindowUniqueName() override;
    void BackBufferResized(
        unsigned width,
        unsigned height,
        unsigned sampleCount) override;
    // bool Begin() override;
    // void End() override;

   private:
    std::vector<unsigned char> buffer;
    std::vector<unsigned char> flipped_buffer;

    bool is_active = false;
    bool is_hovered = false;

    void GetFrameBuffer();
    void DrawMenuBar();
    void DrawFrame();

    float drag_distSince_last_release = 0.0;
    void ProcessInputEvents();

   protected:
    // bool JoystickButtonUpdate(int button, bool pressed) override;
    // bool JoystickAxisUpdate(int axis, float value) override;
    // bool KeyboardUpdate(int key, int scancode, int action, int mods);
    // override; bool MousePosUpdate(double xpos, double ypos) override;
    // bool MouseScrollUpdate(double xoffset, double yoffset) override; bool
    // MouseButtonUpdate(int button, int action, int mods) override; void
    // Animate(float elapsed_time_seconds) override;
    std::unique_ptr<PolyscopeRenderPrivateData> data_;
    std::string child_window_name = "";
};
USTC_CG_NAMESPACE_CLOSE_SCOPE

/*
 * polyscope::draw() 中用于控制镜头的方法
 * processInputEvents();
 * view::updateFlight();
 * showDelayedWarnings();
 */
