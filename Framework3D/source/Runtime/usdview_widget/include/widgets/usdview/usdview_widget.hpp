#pragma once

#include <memory>

#include "GUI/widget.h"
#include "pxr/base/tf/token.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usdImaging/usdImagingGL/engine.h"
#include "stage/stage.hpp"
#include "widgets/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class BaseCamera;
class FreeCamera;
class NodeTree;

struct UsdviewEnginePrivateData;

class USDVIEW_WIDGET_API UsdviewEngine final : public IWidget {
   public:
    explicit UsdviewEngine(Stage* stage);
    void ChooseRenderer(
        const pxr::TfTokenVector& available_renderers,
        unsigned i);
    ~UsdviewEngine() override;
    bool BuildUI() override;
    void SetEditMode(bool editing);

    const void* emit_create_renderer_ui_control()
    {
        auto temp = renderer_ui_control;
        renderer_ui_control = nullptr;
        return temp;
    }

    pxr::VtValue get_renderer_setting(const pxr::TfToken& id) const;
    void set_renderer_setting(
        const pxr::TfToken& id,
        const pxr::VtValue& value);
    void finish_render();

   protected:
    ImGuiWindowFlags GetWindowFlag() override;
    const char* GetWindowName() override;
    std::string GetWindowUniqueName() override;

   private:
    void RenderBackBufferResized(float x, float y);

    enum class CamType { First, Third };
    struct Status {
        CamType cam_type =
            CamType::First;  // 0 for 1st personal, 1 for 3rd personal
        unsigned renderer_id = 0;
    } engine_status;

    bool is_editing_ = false;
    bool is_active = false;
    bool is_hovered = false;

    std::unique_ptr<BaseCamera> free_camera_;
    std::unique_ptr<pxr::UsdImagingGLEngine> renderer_;
    pxr::UsdImagingGLRenderParams _renderParams;
    pxr::GfVec2i render_buffer_size_;

    Stage* stage_;
    pxr::HgiUniquePtr hgi;
    std::vector<uint8_t> texture_data_;
    const void* renderer_ui_control = nullptr;
    bool first_draw = true;
    pxr::TfHashMap<pxr::TfToken, pxr::VtValue, pxr::TfHash> settings;

    void DrawMenuBar();
    void OnFrame(float delta_time);
    void time_controller();

    static void CreateGLContext();

   protected:
    bool JoystickButtonUpdate(int button, bool pressed) override;
    bool JoystickAxisUpdate(int axis, float value) override;
    bool KeyboardUpdate(int key, int scancode, int action, int mods) override;
    bool MousePosUpdate(double xpos, double ypos) override;
    bool MouseScrollUpdate(double xoffset, double yoffset) override;
    bool MouseButtonUpdate(int button, int action, int mods) override;
    void Animate(float elapsed_time_seconds) override;

    void copy_to_presentation();

    std::unique_ptr<UsdviewEnginePrivateData> data_;

    float timecode = 0;
    float time_code_max = 250;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE
