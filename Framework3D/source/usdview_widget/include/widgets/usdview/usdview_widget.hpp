#pragma once

#include <memory>

#include "GUI/widget.h"
#include "nvrhi/nvrhi.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usdImaging/usdImagingGL/engine.h"
#include "widgets/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class BaseCamera;
class FreeCamera;
class NodeTree;
class USDVIEW_WIDGET_API UsdviewEngine final : public IWidget {
   public:
    explicit UsdviewEngine(pxr::UsdStageRefPtr root_stage);
    ~UsdviewEngine() override;
    bool BuildUI(
        // NodeTree* render_node_tree = nullptr,
        // NodeTreeExecutor* get_executor = nullptr
        ) override;
    float current_time_code();
    void set_current_time_code(float time_code);
    // std::unique_ptr<USTC_CG::PickEvent> get_pick_event();
    void set_edit_mode(bool editing);

   private:
    enum class CamType { First, Third };
    struct Status {
        CamType cam_type =
            CamType::First;  // 0 for 1st personal, 1 for 3rd personal
        unsigned renderer_id = 0;
    } engine_status;

    nvrhi::Format present_format = nvrhi::Format::RGBA16_UNORM;

    float timecode = 0;
    float frame_per_second;
    const float time_code_max = 250;
    bool playing = false;
    bool is_editing = false;

    std::unique_ptr<BaseCamera> free_camera_;
    bool is_hovered_ = false;
    std::unique_ptr<pxr::UsdImagingGLEngine> renderer_;
    pxr::UsdImagingGLRenderParams _renderParams;
    pxr::GfVec2i renderBufferSize_;
    bool is_active_;

    pxr::UsdStageRefPtr root_stage_;
    pxr::HgiUniquePtr hgi;
    nvrhi::TextureHandle nvrhi_texture = nullptr;
    std::vector<uint8_t> texture_data;

    void DrawMenuBar();
    void OnFrame(float delta_time);

    static void CreateGLContext();
    // void refresh_viewport(int x, int y);
    // void OnResize(int x, int y);
    //  void time_controller(float delta_time);

   protected:
    bool JoystickButtonUpdate(int button, bool pressed) override;
    bool JoystickAxisUpdate(int axis, float value) override;
    bool KeyboardUpdate(int key, int scancode, int action, int mods) override;
    bool KeyboardCharInput(unsigned unicode, int mods) override;
    bool MousePosUpdate(double xpos, double ypos) override;
    bool MouseScrollUpdate(double xoffset, double yoffset) override;
    bool MouseButtonUpdate(int button, int action, int mods) override;
    void Animate(float elapsed_time_seconds) override;
    void BackBufferResized(
        unsigned width,
        unsigned height,
        unsigned sampleCount) override;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE
