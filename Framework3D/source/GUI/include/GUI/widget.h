#pragma once

#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class Window;

class USTC_CG_API IWidget {
   public:
    IWidget()
    {
    }
    virtual ~IWidget() = default;
    virtual bool BuildUI() = 0;

   protected:
    virtual bool JoystickButtonUpdate(int button, bool pressed);
    virtual bool JoystickAxisUpdate(int axis, float value);
    virtual bool KeyboardUpdate(int key, int scancode, int action, int mods);
    virtual bool KeyboardCharInput(unsigned unicode, int mods);
    virtual bool MousePosUpdate(double xpos, double ypos);
    virtual bool MouseScrollUpdate(double xoffset, double yoffset);
    virtual bool MouseButtonUpdate(int button, int action, int mods);
    virtual void Animate(float elapsed_time_seconds);

    // Give a widget ability to create another widget.
    Window* window;

   private:
    friend class Window;
    friend class DockingImguiRenderer;
    void set_window(Window* window);
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
