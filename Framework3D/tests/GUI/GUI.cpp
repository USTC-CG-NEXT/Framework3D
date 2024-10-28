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
    void BuildUI() override
    {
        ImGui::ShowDemoWindow();
    }
};

TEST(CreateRHI, widget)
{
    Window window;
    std::unique_ptr<IWidget> widget = std::make_unique<Widget>();
    window.register_widget(std::move(widget));
    window.run();
}
