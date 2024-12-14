#include <gtest/gtest.h>

#include "GUI/widget.h"
#include "GUI/window.h"
#include "imgui.h"
using namespace USTC_CG;
TEST(CreateRHI, window)
{
    Window window;
    window.run();
}

class Widget : public IWidget {
   public:
    explicit Widget(const char* title) : title(title)
    {
    }

    bool BuildUI() override
    {
        ImGui::Begin(title.c_str());
        ImGui::Text("Hello, world!");
        ImGui::End();
        // ImGui::ShowDemoWindow();
        return true;
    }

   private:
    std::string title;
};

TEST(CreateRHI, widget)
{
    Window window;
    std::unique_ptr<IWidget> widget = std::make_unique<Widget>("widget");
    window.register_widget(std::move(widget));
    window.run();
}

TEST(CreateRHI, multiple_widgets)
{
    Window window;
    window.register_widget(std::make_unique<Widget>("widget"));
    window.register_widget(std::make_unique<Widget>("widget2"));
    window.run();
}

#include "GUI/ImGuiFileDialog.h"

class FileWidget : public IWidget {
   public:
    explicit FileWidget(const char* title) : title(title)
    {
    }

    bool BuildUI() override
    {
        auto instance = IGFD::FileDialog::Instance();

        instance->Display("SelectFile");

        return true;
    }

   protected:
    const char* GetWindowName() override
    {
        return title.c_str();
    }

   private:
    std::string title;
};

// TEST(FileDialog, create_dialog)
int main()
{
    Window window;
    window.register_widget(std::make_unique<FileWidget>("file"));
    window.run();
}