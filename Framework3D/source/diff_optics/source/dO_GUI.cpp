#include "dO_GUI.hpp"

#include <GUI/widget.h>
#include <diff_optics/api.h>

#include <memory>

#include "diff_optics/lens_system.hpp"
#include "lens_system_gui.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
DiffOpticsGUI::DiffOpticsGUI(LensSystem* lens_system)
    : lens_system(lens_system),
      lens_gui(std::make_unique<LensSystemGUI>(lens_system))
{
}

bool DiffOpticsGUI::BuildUI()
{
    FirstUseEver();

    ImGui::Begin("Lens Canvas", nullptr, ImGuiWindowFlags_None);
    draw_list = ImGui::GetWindowDrawList();
    window_pos = ImGui::GetWindowPos();
    lens_gui->set_canvas_size(
        ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
    lens_gui->draw(this);
    ImGui::End();

    FirstUseEver();

    ImGui::Begin("Lens Controller", nullptr, ImGuiWindowFlags_None);
    draw_list = ImGui::GetWindowDrawList();
    window_pos = ImGui::GetWindowPos();
    lens_gui->control(this);
    ImGui::End();

    return true;
}

std::unique_ptr<IWidget> createDiffOpticsGUI(LensSystem* system)
{
    return std::make_unique<DiffOpticsGUI>(system);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
