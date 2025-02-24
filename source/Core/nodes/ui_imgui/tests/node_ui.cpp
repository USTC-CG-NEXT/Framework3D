
#include "../../system/tests/test_node/test_payload.hpp"
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

        auto loaded = system_->load_configuration("test_nodes.json");

        system_->node_tree_descriptor()->add_socket_group_syncronization(
            { { "simulation_in", "Simulation In", PinKind::Input },
              { "simulation_in", "Simulation Out", PinKind::Output },
              { "simulation_out", "Simulation In", PinKind::Input },
              { "simulation_out", "Simulation Out", PinKind::Output } });

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
    Window window;

    FileBasedNodeWidgetSettings widget_desc;
    widget_desc.system = system_;
    widget_desc.json_path = "testtest.json";
    std::unique_ptr<IWidget> node_widget =
        std::move(create_node_imgui_widget(widget_desc));

    window.register_widget(std::move(node_widget));
    window.run();
}

int main()
{
    std::shared_ptr<NodeSystem> system_;
    log::SetMinSeverity(Severity::Info);
    log::EnableOutputToConsole(true);

    system_ = create_dynamic_loading_system();

    auto loaded = system_->load_configuration("test_nodes.json");

    system_->init();

    Window window;

    FileBasedNodeWidgetSettings widget_desc;
    widget_desc.system = system_;
    system_->set_node_tree_executor(create_node_tree_executor({}));
    widget_desc.json_path = "testtest.json";
    std::unique_ptr<IWidget> node_widget =
        std::move(create_node_imgui_widget(widget_desc));

    system_->get_node_tree_executor()->get_global_payload<TestGlobalPayload&>();

    window.register_function_before_frame([system_](Window* window) {
        if (system_->get_node_tree()->GetDirty()) {
            system_->get_node_tree_executor()
                ->get_global_payload<TestGlobalPayload&>() = { false };
        }
        else {
            system_->get_node_tree_executor()
                ->get_global_payload<TestGlobalPayload&>() = { true };
            system_->get_node_tree()->SetDirty(true);
        }
    });

    window.register_widget(std::move(node_widget));
    window.run();
}