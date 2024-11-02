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
    bool BuildUI() override;
    void SetEditMode(bool editing);

   protected:
    ImGuiWindowFlags GetWindowFlag() override;
    const char* GetWindowName() override;

private:
    void RenderBackBufferResized(float x, float y);

    enum class CamType { First, Third };
    struct Status {
        CamType cam_type =
            CamType::First;  // 0 for 1st personal, 1 for 3rd personal
        unsigned renderer_id = 0;
    } engine_status;

    nvrhi::Format present_format = nvrhi::Format::RGBA16_UNORM;

    bool is_editing_ = false;
    bool is_active = false;
    bool is_hovered = false;

    std::unique_ptr<BaseCamera> free_camera_;
    std::unique_ptr<pxr::UsdImagingGLEngine> renderer_;
    pxr::UsdImagingGLRenderParams _renderParams;
    pxr::GfVec2i render_buffer_size_;

    pxr::UsdStageRefPtr root_stage_;
    pxr::HgiUniquePtr hgi;
    nvrhi::TextureHandle nvrhi_texture_ = nullptr;
    std::vector<uint8_t> texture_data_;

    void DrawMenuBar();
    void OnFrame(float delta_time);

    static void CreateGLContext();

   protected:
    bool JoystickButtonUpdate(int button, bool pressed) override;
    bool JoystickAxisUpdate(int axis, float value) override;
    bool KeyboardUpdate(int key, int scancode, int action, int mods) override;
    bool MousePosUpdate(double xpos, double ypos) override;
    bool MouseScrollUpdate(double xoffset, double yoffset) override;
    bool MouseButtonUpdate(int button, int action, int mods) override;
    void Animate(float elapsed_time_seconds) override;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE
