#define IMGUI_DEFINE_MATH_OPERATORS

#include "GUI/widget.h"

#include "RHI/DeviceManager/DeviceManager.h"
#include "imgui.h"
#include "imgui_internal.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
bool IWidget::Begin()
{
    FirstUseEver();

    auto ret =
        ImGui::Begin(GetWindowUniqueName().c_str(), &is_open, GetWindowFlag());
    draw_list = ImGui::GetWindowDrawList();
    window_pos = ImGui::GetWindowPos();
    return ret;
}

void IWidget::End()
{
    ImGui::End();
}

bool IWidget::IsOpen()
{
    return is_open;
}

void IWidget::BackBufferResized(
    unsigned width,
    unsigned height,
    unsigned sampleCount)
{
}

bool IWidget::JoystickButtonUpdate(int button, bool pressed)
{
    return false;
}

bool IWidget::JoystickAxisUpdate(int axis, float value)
{
    return false;
}

bool IWidget::KeyboardUpdate(int key, int scancode, int action, int mods)
{
    return false;
}

bool IWidget::KeyboardCharInput(unsigned unicode, int mods)
{
    return false;
}

bool IWidget::MousePosUpdate(double xpos, double ypos)
{
    return false;
}

bool IWidget::MouseScrollUpdate(double xoffset, double yoffset)
{
    return false;
}

bool IWidget::MouseButtonUpdate(int button, int action, int mods)
{
    return false;
}

void IWidget::Animate(float elapsed_time_seconds)
{
}

IWidget::~IWidget()
{
    call_back_ = nullptr;
}

void IWidget::SetCallBack(
    const std::function<void(Window*, IWidget*)>& call_back)
{
    this->call_back_ = call_back;
}

void IWidget::CallBack()
{
    if (call_back_) {
        call_back_(window, this);
    }
}

unsigned IWidget::Width() const
{
    return width;
}

unsigned IWidget::Height() const
{
    return height;
}

void IWidget::FirstUseEver() const
{
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_FirstUseEver);
}

const char* IWidget::GetWindowName()
{
    return "Widget";
}

std::string IWidget::GetWindowUniqueName()
{
    return GetWindowName() + std::string("##") +
           std::to_string(reinterpret_cast<uint64_t>(this));
}

ImGuiWindowFlags IWidget::GetWindowFlag()
{
    return ImGuiWindowFlags_None;
}

void IWidget::SetWindow(Window* window)
{
    this->window = window;
}

void IWidget::SetStatus()
{
    size_changed = false;
    if (width != ImGui::GetWindowWidth() ||
        height != ImGui::GetWindowHeight()) {
        size_changed = true;
        width = ImGui::GetWindowWidth();
        height = ImGui::GetWindowHeight();
    }
}

bool IWidget::SizeChanged()
{
    return size_changed;
}

void IWidget::SetNodeSystemDirty(bool dirty)
{
}

void IWidgetDrawable::DrawCircle(
    ImVec2 center,
    float radius,
    float thickness,
    ImColor color,
    int segments)
{
    // draw a circle in the window
    draw_list->AddCircle(
        center + window_pos, radius, color, segments, thickness);
}

void IWidgetDrawable::DrawLine(
    ImVec2 p1,
    ImVec2 p2,
    float thickness,
    ImColor color)
{
    // draw a line in the window
    draw_list->AddLine(p1 + window_pos, p2 + window_pos, color, thickness);
}

void IWidgetDrawable::DrawRect(
    ImVec2 p1,
    ImVec2 p2,
    float thickness,
    ImColor color)
{
    // draw a rectangle in the window
    draw_list->AddRect(
        p1 + window_pos, p2 + window_pos, color, 0, 0, thickness);
}

void IWidgetDrawable::DrawArc(
    ImVec2 center,
    float radius,
    float a_min,
    float a_max,
    float thickness,
    ImColor color,
    int segments)
{
    draw_list->PathArcTo(center + window_pos, radius, a_min, a_max, segments);
    draw_list->PathStroke(color, false, thickness);
}

void IWidgetDrawable::DrawFunction(
    const std::function<float(float)>& f,
    ImVec2 range,
    ImVec2 origin_pos)
{
    // draw a function in the window
    const float step = 2.f;
    ImVec2 p1 = ImVec2(range.x, -f(range.x)) + origin_pos;
    for (float x = range.x + step; x <= range.y; x += step) {
        ImVec2 p2 = ImVec2(x, -f(x)) + origin_pos;

        DrawLine(p1, p2);
        p1 = p2;
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE