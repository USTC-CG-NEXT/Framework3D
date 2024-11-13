#include <GUI/window.h>
#include <gtest/gtest.h>

#include <diff_optics/diff_optics.hpp>

#include "diff_optics/lens_system.hpp"

using namespace USTC_CG;

TEST(dO_GUI_T, diff_optics_gui)
{
    Window window;

    LensSystem lens_system;
    lens_system.add_lens(std::make_shared<NullLayer>(0, 0));
    lens_system.add_lens(std::make_shared<Pupil>(1.f, 10, 0));
    lens_system.add_lens(std::make_shared<LensFilm>(10.f, 5.f, 12.f, 0));
    lens_system.add_lens(std::make_shared<Pupil>(3.f, 15, 0));

    lens_system.add_lens(std::make_shared<Pupil>(0.1f, 20, 0));

    lens_system.add_lens(std::make_shared<NullLayer>(30.f, 0));

    auto gui = createDiffOpticsGUI(&lens_system);

    window.register_widget(std::move(gui));

    window.run();
}

