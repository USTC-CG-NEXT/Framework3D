#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <array>
#include <vector>

#include "glm/fwd.hpp"
#include "imgui_internal.h"
#include "polyscope/curve_network.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/transformation_gizmo.h"
#include "polyscope/types.h"

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
#include "polyscope_widget/polyscope_renderer.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

struct PolyscopeRenderPrivateData {
    nvrhi::TextureDesc nvrhi_desc = {};
    nvrhi::TextureHandle nvrhi_texture = nullptr;
    nvrhi::TextureHandle nvrhi_staging = nullptr;
    nvrhi::Format present_format = nvrhi::Format::RGBA8_UNORM;
    nvrhi::StagingTextureHandle staging_texture;
};

// int nPts = 2000;
// float anotherParam = 0.0;

// void testSubroutine()
// {
//     // do something useful...

//     // Register a structure
//     std::vector<glm::vec3> points;
//     for (int i = 0; i < nPts; i++) {
//         points.push_back(glm::vec3{ polyscope::randomUnit(),
//                                     polyscope::randomUnit(),
//                                     polyscope::randomUnit() });
//     }
//     polyscope::registerPointCloud("my point cloud", points);
// }

// Your callback functions
// void testCallback()
// {
//     // Since options::openImGuiWindowForUserCallback == true by default,
//     // we can immediately start using ImGui commands to build a UI

//     ImGui::PushItemWidth(100);  // Make ui elements 100 pixels wide,
//                                 // instead of full width. Must have
//                                 // matching PopItemWidth() below.

//     ImGui::InputInt("num points", &nPts);             // set a int variable
//     ImGui::InputFloat("param value", &anotherParam);  // set a float variable

//     if (ImGui::Button("run subroutine")) {
//         // executes when button is pressed
//         testSubroutine();
//     }
//     ImGui::SameLine();
//     if (ImGui::Button("hi")) {
//         polyscope::warning("hi");
//     }

//     ImGui::PopItemWidth();
// }

PolyscopeRenderer::PolyscopeRenderer()
{
    data_ = std::make_unique<PolyscopeRenderPrivateData>();
    // polyscope::options::buildGui = false;
    polyscope::options::enableRenderErrorChecks = true;
    polyscope::init();
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
    // Deconstruct slice planes here
    while (!polyscope::state::slicePlanes.empty()) {
        polyscope::state::slicePlanes.pop_back();
    }

    // The deconstruction is complex, I only deconstruct slice planes here to
    // avoid exceptions.
    // The polyscope::shutdown() function does not deconstruct global variables

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

    if (buffer.size() == 0) {
        return false;
    }

    DrawFrame();
    if (enable_input_events) {
        ProcessInputEvents();
    }
    else {
        Set2dMode();
    }
    polyscope::view::updateFlight();
    // polyscope::buildUserGuiAndInvokeCallback();
    return true;
}

std::string PolyscopeRenderer::GetChildWindowName()
{
    return child_window_name;
}

void PolyscopeRenderer::Set2dMode()
{
    // Set the navigation style to 2D
    polyscope::view::setNavigateStyle(polyscope::NavigateStyle::Planar);
    // Set the projection mode to orthographic
    polyscope::view::projectionMode = polyscope::ProjectionMode::Orthographic;
    // Set the view to the XY plane
    polyscope::view::setUpDir(polyscope::view::UpDir::YUp);
    polyscope::view::setFrontDir(polyscope::FrontDir::NegZFront);
    // Reset the camera view to the home view
    polyscope::view::resetCameraToHomeView();
    // Disable the input events
    enable_input_events = false;
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

void PolyscopeRenderer::BackBufferResized(
    unsigned width,
    unsigned height,
    unsigned sampleCount)
{
    IWidget::BackBufferResized(width, height, sampleCount);
    data_->nvrhi_texture = nullptr;
    data_->staging_texture = nullptr;
}

void PolyscopeRenderer::GetFrameBuffer()
{
    buffer = polyscope::screenshotToBufferCustom(false);
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

    if (!data_->nvrhi_texture) {
        std::tie(data_->nvrhi_texture, data_->staging_texture) =
            RHI::load_texture(data_->nvrhi_desc, flipped_buffer.data());
    }
    else {
        RHI::write_texture(
            data_->nvrhi_texture,
            data_->staging_texture,
            flipped_buffer.data());
    }
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
    // DrawMenuBar();

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

// 根据选中的东西的内容，创建一个polyscope::structure，用于突出选中的内容
void PolyscopeRenderer::VisualizePickResult(
    std::pair<polyscope::Structure*, size_t> pickResult)
{
    // 若选中的东西和currPickStructure相同，则不做任何操作
    // if (currPickStructure != nullptr &&
    //     pickResult.first == currPickStructure) {
    //     return;
    // }

    // 若选中的东西为空，则删除当前的polyscope::structure
    if (pickResult.first == nullptr) {
        if (currPickStructure != nullptr) {
            currPickStructure->remove();
            currPickStructure = nullptr;
        }
    }
    else {
        // 若选中的东西不为空，则创建一个polyscope::structure
        if (currPickStructure != nullptr) {
            if (currPickStructure == pickResult.first) {
                return;
            }
            currPickStructure->remove();
        }
        // 得到选中的东西的类型
        auto type = pickResult.first->typeName();
        auto transform = pickResult.first->getTransform();
        if (type == "Surface Mesh") {
            // 检查选中的是顶点、面、边、半边还是角
            auto mesh = dynamic_cast<polyscope::SurfaceMesh*>(pickResult.first);
            auto ind = pickResult.second;
            if (ind < mesh->nVertices()) {
                // 获取顶点坐标
                auto pos = mesh->vertexPositions.getValue(ind);
                // 创建一个点云
                std::vector<glm::vec3> points;
                points.push_back(pos);
                currPickStructure =
                    polyscope::registerPointCloud("picked point", points);
            }
            else if (ind < mesh->nVertices() + mesh->nFaces()) {
                // 获取面的顶点索引和顶点坐标
                ind = ind - mesh->nVertices();
                auto start = mesh->faceIndsStart[ind];
                auto D = mesh->faceIndsStart[ind + 1] - start;
                std::vector<glm::vec3> vertices;
                for (size_t j = 0; j < D; j++) {
                    auto iV = mesh->faceIndsEntries[start + j];
                    vertices.push_back(mesh->vertexPositions.getValue(iV));
                }
                // 创建一个折线
                currPickStructure = polyscope::registerCurveNetworkLoop(
                    "picked face", vertices);
            }
            else if (
                ind < mesh->nVertices() + mesh->nFaces() + mesh->nEdges()) {
                // TODO
            }
            else if (
                ind < mesh->nVertices() + mesh->nFaces() + mesh->nEdges() +
                          mesh->nHalfedges()) {
                // TODO
            }
            else {
                // TODO
            }
        }
        else if (type == "Point Cloud") {
            // TODO
        }
        else if (type == "Curve Network") {
            // TODO
        }
        else {
            // TODO
        }
        if (currPickStructure != nullptr) {
            currPickStructure->setTransform(transform);
        }
    }
}

// Rewritten from processInputEvents() in polyscope.cpp
void PolyscopeRenderer::ProcessInputEvents()
{
    ImGuiIO& io = ImGui::GetIO();

    // If any mouse button is pressed, trigger a redraw
    if (ImGui::IsAnyMouseDown()) {
        // requestRedraw();
        polyscope::requestRedraw();
    }

    ImGuiWindow* window = ImGui::FindWindowByName(child_window_name.c_str());
    glm::vec2 windowPos{ 0., 0. };
    if (window) {
        ImVec2 p = window->Pos;
        windowPos = glm::vec2{ p.x, p.y };
    }

    // Handle transformation gizmo interactions
    bool widgetCapturedMouse = false;
    if (is_hovered) {
        for (polyscope::WeakHandle<polyscope::Widget> wHandle :
             polyscope::state::widgets) {
            if (wHandle.isValid()) {
                polyscope::Widget& w = wHandle.get();
                polyscope::TransformationGizmo* tg =
                    dynamic_cast<polyscope::TransformationGizmo*>(&w);
                if (tg) {
                    widgetCapturedMouse = tg->interactCustom(windowPos);
                    if (widgetCapturedMouse) {
                        break;
                    }
                }
            }
        }
    }

    // Handle scroll events for 3D view
    if (polyscope::state::doDefaultMouseInteraction && !widgetCapturedMouse) {
        // if (!io.WantCaptureMouse && !widgetCapturedMouse) {
        if (is_active && is_hovered) {
            double xoffset = io.MouseWheelH;
            double yoffset = io.MouseWheel;

            if (xoffset != 0 || yoffset != 0) {
                polyscope::requestRedraw();

                // On some setups, shift flips the scroll direction, so take
                // the max scrolling in any direction
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
                    // ImVec2 p = ImGui::GetMousePos();
                    ImVec2 p = ImGui::GetMousePos() - window->Pos;
                    std::pair<polyscope::Structure*, size_t> pickResult =
                        polyscope::pick::pickAtScreenCoords(
                            glm::vec2{ p.x, p.y });
                    polyscope::pick::setSelection(pickResult);
                    // VisualizePickResult(pickResult);
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