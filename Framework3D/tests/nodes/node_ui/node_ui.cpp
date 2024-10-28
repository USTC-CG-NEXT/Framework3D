
#include "GUI/window.h"
#include "gtest/gtest.h"
#include "imgui.h"
#include "nodes/core/node_tree.hpp"
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
        register_cpp_type<int>();
        register_cpp_type<float>();
        register_cpp_type<std::string>();

        NodeTreeDescriptor descriptor;

        // register adding node

        NodeTypeInfo add_node;
        add_node.id_name = "add";
        add_node.ui_name = "Add";
        add_node.ALWAYS_REQUIRED = true;
        add_node.set_declare_function([](NodeDeclarationBuilder& b) {
            b.add_input<int>("a");
            b.add_input<int>("b");
            b.add_output<int>("result");
        });

        add_node.set_execution_function([](ExeParams params) {
            auto a = params.get_input<int>("a");
            auto b = params.get_input<int>("b");
            params.set_output("result", a + b);
        });

        descriptor.register_node(add_node);

        tree = create_node_tree(descriptor);
    }

    void TearDown() override
    {
        entt::meta_reset();
    }
    std::unique_ptr<NodeTree> tree;
};

TEST_F(CreateWindowTest, create_window)
{
    USTC_CG::Window window;

    std::unique_ptr<IWidget> node_widget =
        std::move(create_node_imgui_widget(tree.get()));

    std::unique_ptr<IWidget> empty_widget = std::make_unique<Widget>("Empty");

    window.register_widget(std::move(empty_widget));
    window.register_widget(std::move(node_widget));
    window.run();
}