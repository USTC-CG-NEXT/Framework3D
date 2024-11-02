#include "GUI/widget.h"

#include "RHI/DeviceManager/DeviceManager.h"
#include "imgui.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
bool IWidget::Begin()
{
    FirstUseEver();
    return ImGui::Begin(
        GetWindowUniqueName().c_str(), &is_open, GetWindowFlag());
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
    return GetWindowName();
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

USTC_CG_NAMESPACE_CLOSE_SCOPE