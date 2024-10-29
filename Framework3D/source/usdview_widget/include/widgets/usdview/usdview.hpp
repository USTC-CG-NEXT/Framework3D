#pragma once

#include <complex.h>

#include <memory>

#include "GUI/widget.h"
#include "USTC_CG.h"
#include "pxr/usd/usd/stage.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class NodeTree;
class UsdviewEngine : public IWidget {
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
        unsigned renderer_id = 1;
    } engine_status;

    float timecode = 0;
    float frame_per_second;
    const float time_code_max = 250;
    bool playing = false;
    bool is_editing = false;

    unsigned fbo = 0;
    unsigned tex = 0;
    std::unique_ptr<FreeCamera> free_camera_;
    bool is_hovered_ = false;
    std::unique_ptr<UsdImagingGLEngine> renderer_;
    UsdImagingGLRenderParams _renderParams;
    GfVec2i renderBufferSize_;
    bool is_active_;

    void DrawMenuBar();
    void
    OnFrame(float delta_time, NodeTree* node_tree, NodeTreeExecutor* executor);
    void refresh_platform_texture();
    void refresh_viewport(int x, int y);
    void OnResize(int x, int y);
    // void time_controller(float delta_time);
    bool CameraCallback(float delta_time);
};
USTC_CG_NAMESPACE_CLOSE_SCOPE
