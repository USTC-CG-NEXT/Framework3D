#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#endif
#include "RHI/rhi.hpp"
#include "imgui.h"
#include "nvrhi/nvrhi.h"
#include "polyscope/options.h"
#include "polyscope/pick.h"
#include "polyscope/point_cloud.h"
#include "polyscope/polyscope.h"
#include "polyscope/render/engine.h"
#include "polyscope/screenshot.h"
#include "polyscope/view.h"
#include "polyscope_widget/api.h"
#include "polyscope_widget/polyscope_widget.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

struct PolyscopeRenderPrivateData {
    nvrhi::TextureDesc nvrhi_desc = {};
    nvrhi::TextureHandle nvrhi_texture = nullptr;
    nvrhi::Format present_format = nvrhi::Format::RGBA8_UNORM;
};

polyscope::CameraParameters default_camera_params{
    polyscope::CameraIntrinsics::fromFoVDegVerticalAndAspect(60, 2.),
    polyscope::CameraExtrinsics::fromVectors(
        glm::vec3{ 0., 0., -1. },  // world-space position
        glm::vec3{ 0., 0., 1. },   // world-space look direction
        glm::vec3{ 0., 1., 0. }    // world-space up direction
        )
};

int nPts = 2000;
float anotherParam = 0.0;

void testSubroutine()
{
    // do something useful...

    // Register a structure
    std::vector<glm::vec3> points;
    for (int i = 0; i < nPts; i++) {
        points.push_back(glm::vec3{ polyscope::randomUnit(),
                                    polyscope::randomUnit(),
                                    polyscope::randomUnit() });
    }
    polyscope::registerPointCloud("my point cloud", points);
}

// Your callback functions
void testCallback()
{
    // Since options::openImGuiWindowForUserCallback == true by default,
    // we can immediately start using ImGui commands to build a UI

    ImGui::PushItemWidth(100);  // Make ui elements 100 pixels wide,
                                // instead of full width. Must have
                                // matching PopItemWidth() below.

    ImGui::InputInt("num points", &nPts);             // set a int variable
    ImGui::InputFloat("param value", &anotherParam);  // set a float variable

    if (ImGui::Button("run subroutine")) {
        // executes when button is pressed
        testSubroutine();
    }
    ImGui::SameLine();
    if (ImGui::Button("hi")) {
        polyscope::warning("hi");
    }

    ImGui::PopItemWidth();
}

PolyscopeRenderer::PolyscopeRenderer(polyscope::CameraParameters prams)
{
    data_ = std::make_unique<PolyscopeRenderPrivateData>();
    // polyscope::options::buildGui = false;
    polyscope::init();
    polyscope::view::setViewToCamera(prams);
    // Test register a structure
    // std::vector<glm::vec3> points;
    // for (int i = 0; i < 2000; i++) {
    //     points.push_back(glm::vec3{ polyscope::randomUnit(),
    //                                 polyscope::randomUnit(),
    //                                 polyscope::randomUnit() });
    // }
    // polyscope::registerPointCloud("my point cloud", points);
    // polyscope::state::userCallback = testCallback;
}

PolyscopeRenderer::~PolyscopeRenderer()
{
    polyscope::shutdown();
}

bool PolyscopeRenderer::BuildUI()
{
    if (size_changed) {
        auto size = ImGui::GetContentRegionAvail();
        polyscope::view::setWindowSize(size.x, size.y);
        buffer.resize(size.x * size.y * 4);
        flipped_buffer.resize(size.x * size.y * 4);
    }

    DrawFrame();
    ProcessInputEvents();
    polyscope::view::updateFlight();
    polyscope::buildUserGuiAndInvokeCallback();
    return true;
}

ImGuiWindowFlags PolyscopeRenderer::GetWindowFlag()
{
    return ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse |
           ImGuiWindowFlags_NoScrollbar;
}

const char* PolyscopeRenderer::GetWindowName()
{
    return "Polyscope Renderer";
}

std::string PolyscopeRenderer::GetWindowUniqueName()
{
    return "Polyscope Renderer";
}

void PolyscopeRenderer::GetFrameBuffer()
{
    buffer = polyscope::screenshotToBuffer(false);
    // 上下翻转。每4个数为一个像素，分别为RGBA；每polyscope::view::windowWidth个像素为一行，行内像素顺序不变
    for (int i = 0; i < polyscope::view::windowHeight; i++) {
        memcpy(
            flipped_buffer.data() + i * polyscope::view::windowWidth * 4,
            buffer.data() + (polyscope::view::windowHeight - i - 1) *
                                polyscope::view::windowWidth * 4,
            polyscope::view::windowWidth * 4);
    }

    data_->present_format = nvrhi::Format::RGBA8_UNORM;

    data_->nvrhi_desc.width = polyscope::view::windowWidth;
    data_->nvrhi_desc.height = polyscope::view::windowHeight;
    data_->nvrhi_desc.format = data_->present_format;
    data_->nvrhi_desc.isRenderTarget = true;

    data_->nvrhi_texture =
        RHI::load_texture(data_->nvrhi_desc, flipped_buffer.data());

    // TODO: RHI::write_texture() unimplemented! Need to be replaced after
    //
    // implementation.
    // if (!data_->nvrhi_texture) {
    //     data_->nvrhi_texture =
    //         RHI::load_texture(data_->nvrhi_desc, buffer.data());
    // }
    // else {
    //     RHI::write_texture(...);
    // }
}

void PolyscopeRenderer::DrawMenuBar()
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Image")) {
                polyscope::screenshot();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void PolyscopeRenderer::DrawFrame()
{
    DrawMenuBar();

    // Display some debug info
    // ImGui::Text(
    //     "Rendered window size: %d x %d",
    //     polyscope::view::windowWidth,
    //     polyscope::view::windowHeight);
    // ImGui::Text("io.WantCaptureMouse: %d", ImGui::GetIO().WantCaptureMouse);
    // ImGui::Text(
    //     "io.WantCaptureKeyboard: %d", ImGui::GetIO().WantCaptureKeyboard);
    // ImGui::Text("num widgets: %d", polyscope::state::widgets.size());

    auto previous = data_->nvrhi_texture.Get();
    GetFrameBuffer();

    ImVec2 imgui_frame_size =
        ImVec2(polyscope::view::windowWidth, polyscope::view::windowHeight);
    ImGui::BeginChild(
        "PolyscopeViewPort", imgui_frame_size, 0, ImGuiWindowFlags_NoMove);
    child_window_name = ImGui::GetCurrentWindow()->Name;
    ImGui::GetIO().WantCaptureMouse = false;
    ImGui::Image(
        static_cast<ImTextureID>(data_->nvrhi_texture.Get()),
        imgui_frame_size,
        ImVec2(0.0f, 0.0f),
        ImVec2(1.0f, 1.0f));
    is_active = ImGui::IsWindowFocused();
    is_hovered = ImGui::IsItemHovered();
    ImGui::GetIO().WantCaptureMouse = true;
    ImGui::EndChild();
}

// Rewriten from processInputEvents() in polyscope.cpp
void PolyscopeRenderer::ProcessInputEvents()
{
    ImGuiIO& io = ImGui::GetIO();

    // If any mouse button is pressed, trigger a redraw
    if (ImGui::IsAnyMouseDown()) {
        // requestRedraw();
        polyscope::requestRedraw();
    }

    // Handle scroll events for 3D view
    if (polyscope::state::doDefaultMouseInteraction) {
        // if (!io.WantCaptureMouse && !widgetCapturedMouse) {
        if (is_active && is_hovered) {
            double xoffset = io.MouseWheelH;
            double yoffset = io.MouseWheel;

            if (xoffset != 0 || yoffset != 0) {
                polyscope::requestRedraw();

                // On some setups, shift flips the scroll direction, so take the
                // max scrolling in any direction
                double maxScroll = xoffset;
                if (std::abs(yoffset) > std::abs(xoffset)) {
                    maxScroll = yoffset;
                }

                // Pass camera commands to the camera
                if (maxScroll != 0.0) {
                    bool scrollClipPlane = io.KeyShift;

                    if (scrollClipPlane) {
                        polyscope::view::processClipPlaneShift(maxScroll);
                    }
                    else {
                        polyscope::view::processZoom(maxScroll);
                    }
                }
            }
        }

        // === Mouse inputs
        // if (!io.WantCaptureMouse && !widgetCapturedMouse) {
        if (is_hovered) {
            // Process drags
            bool dragLeft = ImGui::IsMouseDragging(0);
            bool dragRight =
                !dragLeft &&
                ImGui::IsMouseDragging(
                    1);  // left takes priority, so only one can be true
            if (dragLeft || dragRight) {
                glm::vec2 dragDelta{
                    io.MouseDelta.x / polyscope::view::windowWidth,
                    -io.MouseDelta.y / polyscope::view::windowHeight
                };
                drag_distSince_last_release += std::abs(dragDelta.x);
                drag_distSince_last_release += std::abs(dragDelta.y);

                // exactly one of these will be true
                bool isRotate = dragLeft && !io.KeyShift && !io.KeyCtrl;
                bool isTranslate =
                    (dragLeft && io.KeyShift && !io.KeyCtrl) || dragRight;
                bool isDragZoom = dragLeft && io.KeyShift && io.KeyCtrl;

                if (isDragZoom) {
                    polyscope::view::processZoom(dragDelta.y * 5);
                }
                if (isRotate) {
                    ImGuiWindow* window =
                        ImGui::FindWindowByName(child_window_name.c_str());
                    glm::vec2 windowPos{ 0., 0. };
                    if (window) {
                        ImVec2 p = window->Pos;
                        windowPos = glm::vec2{ p.x, p.y };
                    }
                    glm::vec2 currPos{ (io.MousePos.x - windowPos.x) /
                                           polyscope::view::windowWidth,
                                       1.0 -
                                           (io.MousePos.y - windowPos.y) /
                                               polyscope::view::windowHeight };

                    currPos = (currPos * 2.0f) - glm::vec2{ 1.0, 1.0 };
                    if (std::abs(currPos.x) <= 1.0 &&
                        std::abs(currPos.y) <= 1.0) {
                        polyscope::view::processRotate(
                            currPos - 2.0f * dragDelta, currPos);
                    }
                }
                if (isTranslate) {
                    polyscope::view::processTranslate(dragDelta);
                }
            }

            // Click picks
            float dragIgnoreThreshold = 0.01;
            if (ImGui::IsMouseReleased(0)) {
                // Don't pick at the end of a long drag
                if (drag_distSince_last_release < dragIgnoreThreshold) {
                    ImVec2 p = ImGui::GetMousePos();
                    std::pair<polyscope::Structure*, size_t> pickResult =
                        polyscope::pick::pickAtScreenCoords(
                            glm::vec2{ p.x, p.y });
                    polyscope::pick::setSelection(pickResult);
                }

                // Reset the drag distance after any release
                drag_distSince_last_release = 0.0;
            }
            // Clear pick
            if (ImGui::IsMouseReleased(1)) {
                if (drag_distSince_last_release < dragIgnoreThreshold) {
                    polyscope::pick::resetSelection();
                }
                drag_distSince_last_release = 0.0;
            }
        }
    }

    // === Key-press inputs
    if (is_active) {
        polyscope::view::processKeyboardNavigation(io);
    }
}

// bool PolyscopeRenderer::JoystickButtonUpdate(int button, bool pressed)
// {
// }

// bool PolyscopeRenderer::JoystickAxisUpdate(int axis, float value)
// {
// }

// bool PolyscopeRenderer::KeyboardUpdate(
//     int key,
//     int scancode,
//     int action,
//     int mods)
// {
// }

// bool PolyscopeRenderer::MousePosUpdate(double xpos, double ypos)
// {
// }

// bool PolyscopeRenderer::MouseScrollUpdate(double xoffset, double yoffset)
// {
// }

// bool PolyscopeRenderer::MouseButtonUpdate(int button, int action, int
// mods)
// {
// }

// void PolyscopeRenderer::Animate(float elapsed_time_seconds)
// {
// }

USTC_CG_NAMESPACE_CLOSE_SCOPE