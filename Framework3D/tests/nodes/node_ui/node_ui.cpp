
#include "GUI/window.h"
#include "gtest/gtest.h"
#include "nodes/ui/imgui.hpp"

TEST(CreateWindow, create_window)
{
    USTC_CG::Window window;
    auto node_widget = USTC_CG::create_node_imgui_widget(nullptr);
    window.register_widget(std::move(node_widget));
    window.run();
}