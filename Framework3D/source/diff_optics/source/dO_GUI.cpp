#include "dO_GUI.hpp"

#include <GUI/widget.h>
#include <diff_optics/api.h>

#include <memory>

#include "diff_optics/lens_system.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
DiffOpticsGUI::DiffOpticsGUI(LensSystem* lens_system)
    : lens_system(lens_system),
      lens_gui(std::make_unique<LensSystemGUI>(lens_system))
{
}

bool DiffOpticsGUI::BuildUI()
{
    lens_gui->set_canvas_size(Width(), Height());
    lens_gui->draw(this);

    // ImGui::ShowDemoWindow();
    return true;
}

std::unique_ptr<IWidget> createDiffOpticsGUI(LensSystem* system)
{
    return std::make_unique<DiffOpticsGUI>(system);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
