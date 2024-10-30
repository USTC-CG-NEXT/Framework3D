#include "GUI/widget.h"

#include "RHI/DeviceManager/DeviceManager.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
void IWidget::BackBufferResized(
    const unsigned width,
    const unsigned height,
    const unsigned sampleCount)
{
    log::error("Not implemented");
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

void IWidget::set_window(Window* window)
{
    this->window = window;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE