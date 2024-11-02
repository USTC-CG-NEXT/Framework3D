#pragma once

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

    [[nodiscard]] unsigned Width() const;
    [[nodiscard]] unsigned Height() const;

   protected:
    virtual bool Begin();

    /**
     * End() is only called if Begin() returns true.
     */
    virtual void End();

    /**
     * AlwaysEnd() called after End(), even if Begin() returned false.
     */
    virtual void AlwaysEnd();


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


    virtual void FirstUseEver() const;

    virtual const char* GetWindowName();
    virtual std::string GetWindowUniqueName();
    virtual ImGuiWindowFlags GetWindowFlag();

    unsigned width = 800;
    unsigned height = 600;

    bool size_changed;
    bool is_open;

   private:
    friend class Window;
    friend class DockingImguiRenderer;
    void SetWindow(Window* window);

    void SetStatus();
    bool SizeChanged();
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
