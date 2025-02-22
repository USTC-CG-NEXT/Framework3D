#pragma once

#include <functional>
#include <memory>
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
    virtual ~IWidget();
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
    ImDrawList* draw_list;
    ImVec2 window_pos;

    virtual void FirstUseEver() const;

    virtual const char* GetWindowName();
    virtual std::string GetWindowUniqueName();
    virtual ImGuiWindowFlags GetWindowFlag();

    unsigned width = 800;
    unsigned height = 600;

    bool size_changed = true;
    virtual bool Begin();

    virtual void SetNodeSystemDirty(bool dirty = true);

   private:
    bool is_open = true;

    friend class Window;
    friend class DockingImguiRenderer;
    void SetWindow(Window* window);

    void SetStatus();
    bool SizeChanged();
};

class GUI_API IWidgetDrawable : public IWidget {
    void DrawCircle(
        ImVec2 center,
        float radius,
        float thickness = 3,
        ImColor color = ImColor(0.9f, 0.9f, 0.9f),
        int segments = 0);

    void DrawLine(
        ImVec2 p1,
        ImVec2 p2,
        float thickness = 3,
        ImColor color = ImColor(0.9f, 0.9f, 0.9f));

    void DrawRect(
        ImVec2 p1,
        ImVec2 p2,
        float thickness = 3,
        ImColor color = ImColor(0.9f, 0.9f, 0.9f));

    void DrawArc(
        ImVec2 center,
        float radius,
        float a_min,
        float a_max,
        float thickness = 3,
        ImColor color = ImColor(0.9f, 0.9f, 0.9f),
        int segments = 0);

    void DrawFunction(
        const std::function<float(float)>& f,
        ImVec2 range,
        ImVec2 origin_pos);
};

class IWidgetFactory {
   public:
    virtual ~IWidgetFactory() = default;

    // When the widget is created, it will be passed a list of other widgets to
    // interact with.
    virtual std::unique_ptr<IWidget> Create(
        const std::vector<std::unique_ptr<IWidget>>& others) = 0;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
