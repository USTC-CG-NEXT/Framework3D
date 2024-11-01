
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "widgets/usdview/usdview_widget.hpp"

#include <pxr/imaging/hd/driver.h>

#include <vulkan/vulkan.hpp>

#include "Logger/Logger.h"
#include "RHI/Hgi/desc_conversion.hpp"
#include "RHI/rhi.hpp"
#include "free_camera.hpp"
#include "imgui.h"
#include "nvrhi/utils.h"
#include "pxr/base/gf/camera.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/imaging/garch/gl.h"
#include "pxr/imaging/garch/glPlatformContext.h"
#include "pxr/imaging/glf/drawTarget.h"
#include "pxr/imaging/hdx/tokens.h"
#include "pxr/imaging/hgi/blitCmds.h"
#include "pxr/imaging/hgi/blitCmdsOps.h"
#include "pxr/imaging/hgi/tokens.h"
//#include "pxr/imaging/hgiVulkan/texture.h"
//#include "pxr/imaging/hgiVulkan/vk_mem_alloc.h"
#include "pxr/pxr.h"
#include "pxr/usd/usd/primRange.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/camera.h"
#include "pxr/usdImaging/usdImagingGL/engine.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class NodeTree;

UsdviewEngine::UsdviewEngine(pxr::UsdStageRefPtr root_stage)
    : root_stage_(root_stage)
{
    // Initialize OpenGL context using WGL
    CreateGLContext();

    // Initialize GLEW or any other OpenGL loader if necessary
    // glewInit();
    // Check OpenGL version
    GarchGLApiLoad();

    pxr::UsdImagingGLEngine::Parameters params;

    // Initialize Vulkan driver
#if USDVIEW_WITH_VULKAN
    hgi = pxr::Hgi::CreateNamedHgi(pxr::HgiTokens->Vulkan);
    pxr::HdDriver hdDriver;
    hdDriver.name = pxr::HgiTokens->renderDriver;
    hdDriver.driver =
        pxr::VtValue(hgi.get());  // Assuming Vulkan driver doesn't need
                                  // additional parameters
    params.driver = hdDriver;
#endif

    renderer_ = std::make_unique<pxr::UsdImagingGLEngine>(params);

    renderer_->SetEnablePresentation(false);
    free_camera_ = std::make_unique<FirstPersonCamera>();
    static_cast<pxr::UsdGeomCamera&>(*free_camera_) =
        pxr::UsdGeomCamera::Define(root_stage_, pxr::SdfPath("/FreeCamera"));

    static_cast<FirstPersonCamera*>(free_camera_.get())
        ->LookAt(
            pxr::GfVec3d{ 0, -1, 0 },
            pxr::GfVec3d{ 0, 0, 0 },
            pxr::GfVec3d{ 0, 0, 1 });

    auto plugins = renderer_->GetRendererPlugins();
    for (const auto& plugin : plugins) {
        log::info(plugin.GetText());
    }
    renderer_->SetRendererPlugin(plugins[engine_status.renderer_id]);

    // free_camera_->SetProjection(GfCamera::Projection::Perspective);
    free_camera_->CreateFocusDistanceAttr().Set(5.0f);
    free_camera_->CreateClippingRangeAttr(
        pxr::VtValue(pxr::GfVec2f{ 0.1f, 1000.f }));
}

void UsdviewEngine::DrawMenuBar()
{
    ImGui::BeginMenuBar();
    if (ImGui::BeginMenu("Free Camera")) {
        if (ImGui::BeginMenu("Camera Type")) {
            if (ImGui::MenuItem(
                    "First Personal",
                    0,
                    this->engine_status.cam_type == CamType::First)) {
                if (engine_status.cam_type != CamType::First) {
                    free_camera_ = std::make_unique<FirstPersonCamera>();
                    engine_status.cam_type = CamType::First;
                }
            }
            if (ImGui::MenuItem(
                    "Third Personal",
                    0,
                    this->engine_status.cam_type == CamType::Third)) {
                if (engine_status.cam_type != CamType::Third) {
                    free_camera_ = std::make_unique<ThirdPersonCamera>();
                    engine_status.cam_type = CamType::Third;
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Renderer")) {
        if (ImGui::BeginMenu("Select Renderer")) {
            auto available_renderers = renderer_->GetRendererPlugins();
            for (unsigned i = 0; i < available_renderers.size(); ++i) {
                if (ImGui::MenuItem(
                        available_renderers[i].GetText(),
                        0,
                        this->engine_status.renderer_id == i)) {
                    if (this->engine_status.renderer_id != i) {
                        renderer_->SetRendererPlugin(available_renderers[i]);

                        // Perform a fake resize event
                        // refresh_viewport(
                        //    renderBufferSize_[0], renderBufferSize_[1]);
                        this->engine_status.renderer_id = i;
                    }
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

void UsdviewEngine::OnFrame(float delta_time)
{
    DrawMenuBar();

    auto previous = nvrhi_texture.Get();

    using namespace pxr;
    GfFrustum frustum =
        free_camera_->GetCamera(UsdTimeCode::Default()).GetFrustum();

    GfMatrix4d projectionMatrix = frustum.ComputeProjectionMatrix();
    GfMatrix4d viewMatrix = frustum.ComputeViewMatrix();

    renderer_->SetCameraState(viewMatrix, projectionMatrix);
    renderer_->SetRendererAov(HdAovTokens->color);
    // renderer_->SetRendererSetting(
    //     TfToken("RenderNodeTree"), VtValue((void*)node_tree));
    // renderer_->SetRendererSetting(
    //     TfToken("RenderNodeTreeExecutor"), VtValue((void*)executor));

    _renderParams.enableLighting = true;
    _renderParams.enableSceneMaterials = true;
    _renderParams.showRender = true;
    _renderParams.frame = UsdTimeCode::Default();
    _renderParams.drawMode = UsdImagingGLDrawMode::DRAW_WIREFRAME_ON_SURFACE;
    _renderParams.colorCorrectionMode = pxr::HdxColorCorrectionTokens->disabled;

    _renderParams.clearColor = GfVec4f(0.2f, 0.2f, 0.2f, 1.f);
    _renderParams.frame = UsdTimeCode(timecode);

    for (int i = 0; i < free_camera_->GetCamera(UsdTimeCode::Default())
                            .GetClippingPlanes()
                            .size();
         ++i) {
        _renderParams.clipPlanes[i] =
            free_camera_->GetCamera(UsdTimeCode::Default())
                .GetClippingPlanes()[i];
    }

    GlfSimpleLightVector lights(1);
    auto cam_pos = frustum.GetPosition();
    lights[0].SetPosition(GfVec4f{
        float(cam_pos[0]), float(cam_pos[1]), float(cam_pos[2]), 1.0f });
    lights[0].SetAmbient(GfVec4f(1, 1, 1, 1));
    lights[0].SetDiffuse(GfVec4f(1.0f) * 1.9);
    GlfSimpleMaterial material;
    float kA = 1.0f;
    float kS = 1.0f;
    float shiness = 1.f;
    material.SetDiffuse(GfVec4f(kA, kA, kA, 1.0f));
    material.SetSpecular(GfVec4f(kS, kS, kS, 1.0f));
    material.SetShininess(shiness);
    GfVec4f sceneAmbient = { 0.01, 0.01, 0.01, 1.0 };
    renderer_->SetLightingState(lights, material, sceneAmbient);
    renderer_->SetRendererAov(HdAovTokens->color);

    UsdPrim root = root_stage_->GetPseudoRoot();

    renderer_->Render(root, _renderParams);
    auto hgi_texture = renderer_->GetAovTexture(HdAovTokens->color);
    nvrhi::TextureDesc tex_desc =
        RHI::ConvertToNvrhiTextureDesc(hgi_texture->GetDescriptor());

    // Since Hgi and nvrhi vulkan are on different Vulkan instances and we don't
    // want to modify Hgi's external information definition, we need to do a CPU
    // read back to send the information to nvrhi.

    HgiBlitCmdsUniquePtr blitCmds = hgi->CreateBlitCmds();
    HgiTextureGpuToCpuOp copyOp;
    copyOp.gpuSourceTexture = hgi_texture;
    copyOp.cpuDestinationBuffer = texture_data.data();
    copyOp.destinationBufferByteSize = texture_data.size();
    blitCmds->CopyTextureGpuToCpu(copyOp);

    hgi->SubmitCmds(blitCmds.get(), HgiSubmitWaitTypeWaitUntilCompleted);

#if USDVIEW_WITH_VULKAN
    nvrhi_texture = RHI::load_texture(tex_desc, texture_data.data());

    // auto img =
    //     static_cast<pxr::HgiVulkanTexture*>((hgi_texture.Get()))->GetImage();

    // nvrhi_texture = RHI::get_device()->createHandleForNativeTexture(
    //     nvrhi::ObjectTypes::VK_Image, img, tex_desc);
#else
    nvrhi_texture = RHI::load_ogl_texture(tex_desc, color->GetRawResource());
#endif

    auto imgui_frame_size = ImVec2(renderBufferSize_[0], renderBufferSize_[1]);

    ImGui::BeginChild("ViewPort", imgui_frame_size, 0, ImGuiWindowFlags_NoMove);

    ImGui::GetIO().WantCaptureMouse = false;
    ImGui::Image(
        static_cast<ImTextureID>(nvrhi_texture.Get()),
        imgui_frame_size,
        ImVec2(0.0f, 1.0f),
        ImVec2(1.0f, 0.0f));
    // is_active_ = ImGui::IsWindowFocused();
    // is_hovered_ = ImGui::IsItemHovered();

    // if (is_hovered_ && is_editing &&
    //     ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
    //     auto mouse_pos_rel = ImGui::GetMousePos() - ImGui::GetItemRectMin();
    //     // Normalize the mouse position to be in the range [0, 1]
    //     ImVec2 mousePosNorm = ImVec2(
    //         mouse_pos_rel.x / renderBufferSize_[0],
    //         mouse_pos_rel.y / renderBufferSize_[1]);

    //    // Convert to NDC coordinates
    //    ImVec2 mousePosNDC =
    //        ImVec2(mousePosNorm.x * 2.0f - 1.0f, 1.0f - mousePosNorm.y
    //        * 2.0f);

    // GfVec3d point;
    // GfVec3d normal;
    // SdfPath path;
    // SdfPath instancer;
    // HdInstancerContext outInstancerContext;
    // int outHitInstanceIndex;
    // auto narrowed = frustum.ComputeNarrowedFrustum(
    //     { mousePosNDC[0], mousePosNDC[1] },
    //     { 1.0 / renderBufferSize_[0], 1.0 / renderBufferSize_[1] });

    // if (renderer_->TestIntersection(
    //         narrowed.ComputeViewMatrix(),
    //         narrowed.ComputeProjectionMatrix(),
    //         root,
    //         _renderParams,
    //         &point,
    //         &normal,
    //         &path,
    //         &instancer,
    //         &outHitInstanceIndex,
    //         &outInstancerContext)) {
    //     pick_event = std::make_unique<PickEvent>(
    //         point,
    //         normal,
    //         path,
    //         instancer,
    //         outInstancerContext,
    //         outHitInstanceIndex,
    //         narrowed.ComputePickRay({ mousePosNDC[0], mousePosNDC[1] }));

    //    log::info("Picked prim " + path.GetAsString(), Info);
    //}
    ImGui::GetIO().WantCaptureMouse = true;

    ImGui::EndChild();
}

//
// void UsdviewEngine::refresh_viewport(int x, int y)
//{
//    renderBufferSize_[0] = x;
//    renderBufferSize_[1] = y;
//
//    renderer_->SetRenderBufferSize(renderBufferSize_);
//    renderer_->SetRenderViewport(GfVec4d{
//        0.0, 0.0, double(renderBufferSize_[0]), double(renderBufferSize_[1])
//        });
//    free_camera_->m_ViewportSize = renderBufferSize_;
//
//    refresh_platform_texture();
//}
//

//
// void UsdviewEngine::time_controller(float delta_time)
//{
//    if (is_active_ && ImGui::IsKeyPressed(ImGuiKey_Space)) {
//        playing = !playing;
//    }
//    if (playing) {
//        timecode += delta_time * GlobalUsdStage::timeCodesPerSecond;
//        if (timecode > time_code_max) {
//            timecode = 0;
//        }
//    }
//
//    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
//    if (ImGui::SliderFloat("Time##timecode", &timecode, 0, time_code_max)) {
//    }
//}

void UsdviewEngine::set_current_time_code(float time_code)
{
    timecode = time_code;
}

// std::unique_ptr<USTC_CG::PickEvent> UsdviewEngine::get_pick_event()
//{
//     return std::move(pick_event);
// }
//
// bool UsdviewEngine::CameraCallback(float delta_time)
//{
//    ImGuiIO& io = ImGui::GetIO();
//    if (is_active_) {
//        free_camera_->KeyboardUpdate();
//    }
//
//    if (is_hovered_) {
//        for (int i = 0; i < 5; ++i) {
//            if (io.MouseClicked[i]) {
//                free_camera_->MouseButtonUpdate(i);
//            }
//        }
//        float fovAdjustment = io.MouseWheel * 5.0f;
//        if (fovAdjustment != 0) {
//            free_camera_->MouseScrollUpdate(fovAdjustment);
//        }
//    }
//    for (int i = 0; i < 5; ++i) {
//        if (io.MouseReleased[i]) {
//            free_camera_->MouseButtonUpdate(i);
//        }
//    }
//    free_camera_->MousePosUpdate(io.MousePos.x, io.MousePos.y);
//
//    free_camera_->Animate(delta_time);
//
//    return false;
//}

bool UsdviewEngine::JoystickButtonUpdate(int button, bool pressed)
{
    free_camera_->JoystickButtonUpdate(button, pressed);
    return false;
}

bool UsdviewEngine::JoystickAxisUpdate(int axis, float value)
{
    free_camera_->JoystickUpdate(axis, value);
    return false;
}

bool UsdviewEngine::KeyboardUpdate(int key, int scancode, int action, int mods)
{
    free_camera_->KeyboardUpdate(key, scancode, action, mods);
    return false;
}

bool UsdviewEngine::KeyboardCharInput(unsigned unicode, int mods)
{
    free_camera_->KeyboardUpdate(unicode, 0, 0, mods);
    return false;
}

bool UsdviewEngine::MousePosUpdate(double xpos, double ypos)
{
    free_camera_->MousePosUpdate(xpos, ypos);
    return false;
}

bool UsdviewEngine::MouseScrollUpdate(double xoffset, double yoffset)
{
    free_camera_->MouseScrollUpdate(xoffset, yoffset);
    return false;
}

bool UsdviewEngine::MouseButtonUpdate(int button, int action, int mods)
{
    free_camera_->MouseButtonUpdate(button, action, mods);
    return false;
}

void UsdviewEngine::Animate(float elapsed_time_seconds)
{
    free_camera_->Animate(elapsed_time_seconds);
    IWidget::Animate(elapsed_time_seconds);
}

void UsdviewEngine::BackBufferResized(
    unsigned width,
    unsigned height,
    unsigned sampleCount)
{
    IWidget::BackBufferResized(width, height, sampleCount);

    renderBufferSize_[0] = width;
    renderBufferSize_[1] = height;

    renderer_->SetRenderBufferSize(renderBufferSize_);
    renderer_->SetRenderViewport(pxr::GfVec4d{
        0.0, 0.0, double(renderBufferSize_[0]), double(renderBufferSize_[1]) });

    texture_data.resize(
        width * height * RHI::calculate_bytes_per_pixel(present_format));
}

void UsdviewEngine::CreateGLContext()
{
    HDC hdc = GetDC(GetConsoleWindow());
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HGLRC hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hglrc);
}

UsdviewEngine::~UsdviewEngine()
{
}

bool UsdviewEngine::BuildUI(
    // NodeTree* render_node_tree,
    // NodeTreeExecutor* get_executor
)
{
    auto delta_time = ImGui::GetIO().DeltaTime;

    // ImGui::SetNextWindowSize({ 800, 600 });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin(
            "UsdView Engine",
            nullptr,
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse)) {
        ImGui::PopStyleVar(1);

        auto size = ImGui::GetContentRegionAvail();
        // size.y -= 28;
        BackBufferResized(size.x, size.y, 1);

        if (size.x > 0 && size.y > 0) {
            OnFrame(delta_time);
            // time_controller(delta_time);
        }
    }
    else {
        ImGui::PopStyleVar(1);
    }
    ImGui::End();
    return true;
}

float UsdviewEngine::current_time_code()
{
    return timecode;
}
//
// void UsdviewEngine::set_current_time_code(float time_code)
//{
//    timecode = time_code;
//}

// std::unique_ptr<PickEvent> UsdviewEngine::get_pick_event()
//{
//     return impl_->get_pick_event();
// }

void UsdviewEngine::set_edit_mode(bool editing)
{
    is_editing = editing;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
