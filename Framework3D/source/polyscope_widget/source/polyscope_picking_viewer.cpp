#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#endif
#include "imgui.h"
#include "polyscope/pick.h"
#include "polyscope/polyscope.h"
#include "polyscope_widget/api.h"
#include "polyscope_widget/polyscope_picking_viewer.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

PolyscopePickingViewer::PolyscopePickingViewer()
{
}

PolyscopePickingViewer::~PolyscopePickingViewer()
{
}

bool PolyscopePickingViewer::BuildUI()
{
    ImGui::Begin("Polyscope Picking Viewer", nullptr, ImGuiWindowFlags_None);
    BuildPickGui();
    ImGui::End();

    return true;
}

// Rewritten from buildPickGui() in polyscope.cpp
void PolyscopePickingViewer::BuildPickGui()
{
    if (polyscope::pick::haveSelection()) {
        std::pair<polyscope::Structure*, size_t> selection =
            polyscope::pick::getSelection();

        ImGui::TextUnformatted(
            (selection.first->typeName() + ": " + selection.first->name)
                .c_str());
        ImGui::Separator();
        selection.first->buildPickUI(selection.second);
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE