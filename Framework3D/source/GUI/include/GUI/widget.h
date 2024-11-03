#pragma once

#include <functional>
#include <string>

#include "GUI/api.h"
#include "imgui.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class Window;

class GUI_API IWidget {
   public:
    IWidget()
    {
    }
    virtual ~IWidget() = default;
    virtual bool BuildUI() = 0;

    virtual void SetCallBack(const std::function<void(Window*, IWidget*)>&);
    void CallBack();

    [[nodiscard]] unsigned Width() const;
    [[nodiscard]] unsigned Height() const;

   protected:
    /**
     * End() is only called if Begin() returns true.
     */
    virtual void End();

    virtual bool IsOpen();

    virtual void
    BackBufferResized(unsigned width, unsigned height, unsigned sampleCount);
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
    std::function<void(Window*, IWidget*)> call_back_;

    virtual void FirstUseEver() const;

    virtual const char* GetWindowName();
    virtual std::string GetWindowUniqueName();
    virtual ImGuiWindowFlags GetWindowFlag();

    unsigned width = 800;
    unsigned height = 600;

    bool size_changed = true;
    virtual bool Begin();

   private:
    bool is_open = true;

    friend class Window;
    friend class DockingImguiRenderer;
    void SetWindow(Window* window);

    void SetStatus();
    bool SizeChanged();
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
