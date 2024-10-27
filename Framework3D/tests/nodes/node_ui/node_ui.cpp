

#include "GUI/window/window.h"
#include "gtest/gtest.h"

TEST(CreateWindow, create_window)
{
    USTC_CG::Window window("Test Window");
    EXPECT_TRUE(window.init());
}