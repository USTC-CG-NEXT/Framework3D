
#include "GUI/window.h"
#include "Logger/Logger.h"
#include "gtest/gtest.h"
#include "imgui.h"
#include "nodes/core/node_tree.hpp"
#include "nodes/system/node_system.hpp"
#include "nodes/ui/imgui.hpp"
using namespace USTC_CG;

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

class CreateWindowTest : public ::testing::Test {
   protected:
    void SetUp() override
    {
        log::SetMinSeverity(Severity::Info);
        log::EnableOutputToConsole(true);

        system_ = create_dynamic_loading_system();
        system_->register_cpp_types<int>();

        auto loaded = system_->load_configuration("test_nodes.json");

        ASSERT_TRUE(loaded);
        system_->init();
    }

    void TearDown() override
    {
        system_.reset();
    }
    std::shared_ptr<NodeSystem> system_;
};

TEST_F(CreateWindowTest, create_window)
{
    USTC_CG::Window window;

    FileBasedNodeWidgetSettings widget_desc;
    widget_desc.system = system_;
    widget_desc.json_path = "testtest.json";
    std::unique_ptr<IWidget> node_widget =
        std::move(create_node_imgui_widget(widget_desc));

    window.register_widget(std::move(node_widget));
    window.run();
}
