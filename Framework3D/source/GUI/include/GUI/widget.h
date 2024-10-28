#pragma once

#include "RHI/DeviceManager/DeviceManager.h"
#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class IWidget : IRenderPass {
   protected:
    // creates the UI in ImGui, updates internal UI state
    virtual void buildUI(void) = 0;

   private:
    // buffer mouse click and keypress events to make sure we don't lose events
    // which last less than a full frame
    std::array<bool, 3> mouseDown = { false };
    std::array<bool, GLFW_KEY_LAST + 1> keyDown = { false };

    IWidget(DeviceManager* devManager);
    ~IWidget() override;

    bool KeyboardUpdate(int key, int scancode, int action, int mods) override;
    bool KeyboardCharInput(unsigned int unicode, int mods) override;
    bool MousePosUpdate(double xpos, double ypos) override;
    bool MouseScrollUpdate(double xoffset, double yoffset) override;
    bool MouseButtonUpdate(int button, int action, int mods) override;
    void Animate(float elapsedTimeSeconds) override;
    void Render(nvrhi::IFramebuffer* framebuffer) override;
    void BackBufferResizing() override;

    void BeginFullScreenWindow();
    void DrawScreenCenteredText(const char* text);
    void EndFullScreenWindow();
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
