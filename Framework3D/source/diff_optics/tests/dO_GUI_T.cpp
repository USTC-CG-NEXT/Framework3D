#include <GUI/window.h>
#include <gtest/gtest.h>

#include <diff_optics/diff_optics.hpp>

#include "diff_optics/lens_system.hpp"

using namespace USTC_CG;

TEST(dO_GUI_T, diff_optics_gui)
{
    Window window;

    LensSystem lens_system;
    lens_system.add_lens(std::make_shared<Pupil>(1.0f, 10, 300));
    auto gui = createDiffOpticsGUI(&lens_system);

    window.register_widget(std::move(gui));

    window.run();
}