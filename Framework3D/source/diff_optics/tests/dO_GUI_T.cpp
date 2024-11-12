#include <GUI/window.h>
#include <gtest/gtest.h>

#include <diff_optics/diff_optics.hpp>

using namespace USTC_CG;

TEST(dO_GUI_T, diff_optics_gui)
{

    Window window;

    auto gui = createDiffOpticsGUI();

    window.register_widget(std::move(gui));

    window.run();
}