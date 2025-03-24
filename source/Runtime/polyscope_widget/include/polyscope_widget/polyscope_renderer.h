#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "GUI/widget.h"
#include "imgui.h"
#include "polyscope/camera_parameters.h"
#include "polyscope/polyscope.h"
#include "polyscope/structure.h"
#include "polyscope/types.h"
#include "polyscope/view.h"
#include "polyscope_widget/api.h"
#include "pxr/base/tf/token.h"
#include "pxr/usd/usd/stage.h"
#include "stage/stage.hpp"
#include "stage_listener/stage_listener.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class BaseCamera;
class FreeCamera;
class NodeTree;

using DirtyPathSet = std::unordered_set<pxr::SdfPath, pxr::SdfPath::Hash>;

class POLYSCOPE_WIDGET_API PolyscopeRenderer final : public IWidget {
   public:
    explicit PolyscopeRenderer(Stage* stage);
    ~PolyscopeRenderer() override;

    bool BuildUI() override;
    // void SetCallBack(const std::function<void(Window*, IWidget*)>&) override;
    // Position, size, is_active, is_hovered
    std::string GetChildWindowName();
    void Set2dMode();

    bool GetInputTransformTriggered() const
    {
        return input_transform_triggered;
    }

    bool GetInputPickTriggered() const
    {
        return input_pick_triggered;
    }

    static std::vector<std::pair<polyscope::Structure*, size_t>> GetPickResult()
    {
        return pick_result;
    }

    static std::vector<size_t> GetControlPoints(
        const std::string& structure_name)
    {
        std::vector<size_t> control_points;
        for (const auto& [pickResult, structure] :
             visualization_structure_map) {
            if (pickResult.first->getName() == structure_name &&
                structure != nullptr) {
                control_points.push_back(pickResult.second);
            }
        }
        return control_points;
    }

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
    Stage* stage_;
    std::vector<unsigned char> buffer;
    std::vector<unsigned char> flipped_buffer;

    bool enable_input_events = true;
    bool input_transform_triggered = false;
    bool input_pick_triggered = false;

    bool is_active = false;
    bool is_hovered = false;

    StageListener stage_listener;
    DirtyPathSet dirty_paths;

    std::chrono::time_point<std::chrono::steady_clock> lastMainLoopIterTime;

    void GetFrameBuffer();
    void DrawMenuBar();
    pxr::UsdGeomXformCache xform_cache;
    void RegisterGeometryFromPrim(const pxr::UsdPrim& prim);
    void UpdateStructures(DirtyPathSet paths);
    void DrawFrame();

    static std::vector<std::pair<polyscope::Structure*, size_t>> pick_result;
    // polyscope::Structure* curr_visualization_structure = nullptr;

    static std::
        map<std::pair<polyscope::Structure*, size_t>, polyscope::Structure*>
            visualization_structure_map;

    void VisualizePickVertexGizmo(
        std::pair<polyscope::Structure*, size_t> pickResult);
    void UpdatePickStructure(
        std::pair<polyscope::Structure*, size_t> pickResult);

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
    std::string child_window_name = "";
};
USTC_CG_NAMESPACE_CLOSE_SCOPE

/*
 * polyscope::draw() 中用于控制镜头的方法
 * processInputEvents();
 * view::updateFlight();
 * showDelayedWarnings();
 */
