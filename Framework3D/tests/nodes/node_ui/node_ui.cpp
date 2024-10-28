
#include "GUI/window.h"
#include "gtest/gtest.h"
#include "nodes/ui/imgui.hpp"
using namespace USTC_CG;

TEST(CreateWindow, create_window)
{
    USTC_CG::Window window;
    NodeTree* tree = nullptr;
    std::unique_ptr<IWidget> node_widget =
        std::move(create_node_imgui_widget(tree));
    window.register_widget(std::move(node_widget));
    window.run();
}