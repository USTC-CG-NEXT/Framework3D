#include "dO_GUI.hpp"

#include <GUI/widget.h>
#include <diff_optics/api.h>

#include <memory>

#include "diff_optics/lens_system.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
bool DiffOpticsGUI::BuildUI()
{
    lens_system->draw(this);

    // ImGui::ShowDemoWindow();
    return true;
}

std::unique_ptr<IWidget> createDiffOpticsGUI(LensSystem* system)
{
    return std::make_unique<DiffOpticsGUI>(system);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
