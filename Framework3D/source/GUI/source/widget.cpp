#include "GUI/widget.h"

#include "RHI/DeviceManager/DeviceManager.h"
#include "RHI/rhi.hpp"
#include "imgui.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

IWidget::IWidget(DeviceManager* devManager) : IRenderPass(devManager)
{
}

IWidget::~IWidget()
{
}
//
// bool IWidget::Init(std::shared_ptr<ShaderFactory> shaderFactory)
//{
//    // Set up keyboard mapping.
//    // ImGui will use those indices to peek into the io.KeyDown[] array
//    // that we will update during the application lifetime.
//    ImGuiIO& io = ImGui::GetIO();
//    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
//    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
//    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
//    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
//    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
//    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
//    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
//    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
//    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
//    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
//    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
//    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
//    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
//    io.KeyMap[ImGuiKey_A] = 'A';
//    io.KeyMap[ImGuiKey_C] = 'C';
//    io.KeyMap[ImGuiKey_V] = 'V';
//    io.KeyMap[ImGuiKey_X] = 'X';
//    io.KeyMap[ImGuiKey_Y] = 'Y';
//    io.KeyMap[ImGuiKey_Z] = 'Z';
//
//    imgui_nvrhi = std::make_unique<ImGui_NVRHI>();
//    return imgui_nvrhi->init(GetDevice(), shaderFactory);
//}
//
// ImFont* IWidget::LoadFont(
//    IFileSystem& fs,
//    const std::filesystem::path& fontFile,
//    float fontSize)
//{
//    auto fontData = fs.readFile(fontFile);
//    if (!fontData)
//        return nullptr;
//
//    ImFontConfig fontConfig;
//
//    // XXXX mk: this appears to be a bug: the atlas copies (& owns) the data
//    // when the flag is set to false !
//    fontConfig.FontDataOwnedByAtlas = false;
//    ImFont* imFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
//        (void*)(fontData->data()),
//        (int)(fontData->size()),
//        fontSize,
//        &fontConfig);
//
//    return imFont;
//}

bool IWidget::KeyboardUpdate(int key, int scancode, int action, int mods)
{
    auto& io = ImGui::GetIO();

    bool keyIsDown;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        keyIsDown = true;
    }
    else {
        keyIsDown = false;
    }

    // update our internal state tracking for this key button
    keyDown[key] = keyIsDown;

    if (keyIsDown) {
        // if the key was pressed, update ImGui immediately
        io.KeysDown[key] = true;
    }
    else {
        // for key up events, ImGui state is only updated after the next frame
        // this ensures that short keypresses are not missed
    }

    return io.WantCaptureKeyboard;
}

bool IWidget::KeyboardCharInput(unsigned int unicode, int mods)
{
    auto& io = ImGui::GetIO();

    io.AddInputCharacter(unicode);

    return io.WantCaptureKeyboard;
}

bool IWidget::MousePosUpdate(double xpos, double ypos)
{
    auto& io = ImGui::GetIO();
    io.MousePos.x = float(xpos);
    io.MousePos.y = float(ypos);

    return io.WantCaptureMouse;
}

bool IWidget::MouseScrollUpdate(double xoffset, double yoffset)
{
    auto& io = ImGui::GetIO();
    io.MouseWheel += float(yoffset);

    return io.WantCaptureMouse;
}

bool IWidget::MouseButtonUpdate(int button, int action, int mods)
{
    auto& io = ImGui::GetIO();

    bool buttonIsDown;
    int buttonIndex;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        buttonIsDown = true;
    }
    else {
        buttonIsDown = false;
    }

    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT: buttonIndex = 0; break;

        case GLFW_MOUSE_BUTTON_RIGHT: buttonIndex = 1; break;

        case GLFW_MOUSE_BUTTON_MIDDLE: buttonIndex = 2; break;
    }

    // update our internal state tracking for this mouse button
    mouseDown[buttonIndex] = buttonIsDown;

    if (buttonIsDown) {
        // update ImGui state immediately
        io.MouseDown[buttonIndex] = true;
    }
    else {
        // for mouse up events, ImGui state is only updated after the next frame
        // this ensures that short clicks are not missed
    }

    return io.WantCaptureMouse;
}

void IWidget::Animate(float elapsedTimeSeconds)
{
    // if (!imgui_nvrhi)
    //     return;

    int w, h;
    float scaleX, scaleY;

    GetDeviceManager()->GetWindowDimensions(w, h);
    GetDeviceManager()->GetDPIScaleInfo(scaleX, scaleY);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(float(w), float(h));
    io.DisplayFramebufferScale.x = scaleX;
    io.DisplayFramebufferScale.y = scaleY;

    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] ||
                 io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift =
        io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt =
        io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper =
        io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void IWidget::Render(nvrhi::IFramebuffer* framebuffer)
{
    buildUI();

    ImGui::Render();
    // imgui_nvrhi->render(framebuffer);

    // reconcile mouse button states
    auto& io = ImGui::GetIO();
    for (size_t i = 0; i < mouseDown.size(); i++) {
        if (io.MouseDown[i] == true && mouseDown[i] == false) {
            io.MouseDown[i] = false;
        }
    }

    // reconcile key states
    for (size_t i = 0; i < keyDown.size(); i++) {
        if (io.KeysDown[i] == true && keyDown[i] == false) {
            io.KeysDown[i] = false;
        }
    }
}

void IWidget::BackBufferResizing()
{
}

void IWidget::BeginFullScreenWindow()
{
    int width, height;
    GetDeviceManager()->GetWindowDimensions(width, height);
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(
        ImVec2(float(width), float(height)), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::SetNextWindowBgAlpha(0.f);
    ImGui::Begin(
        " ",
        0,
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar);
}

void IWidget::DrawScreenCenteredText(const char* text)
{
    int width, height;
    GetDeviceManager()->GetWindowDimensions(width, height);
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImGui::SetCursorPosX((float(width) - textSize.x) * 0.5f);
    ImGui::SetCursorPosY((float(height) - textSize.y) * 0.5f);
    ImGui::TextUnformatted(text);
}

void IWidget::EndFullScreenWindow()
{
    ImGui::End();
    ImGui::PopStyleVar();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE