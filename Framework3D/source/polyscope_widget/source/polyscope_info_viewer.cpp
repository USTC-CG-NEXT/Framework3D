#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#endif
#include "imgui.h"
#include "polyscope/polyscope.h"
#include "polyscope_widget/api.h"
#include "polyscope_widget/polyscope_info_viewer.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

PolyscopeInfoViewer::PolyscopeInfoViewer()
{
}

PolyscopeInfoViewer::~PolyscopeInfoViewer()
{
}

bool PolyscopeInfoViewer::BuildUI()
{
    ImGui::Begin("Polyscope Info", nullptr, ImGuiWindowFlags_None);
    polyscope::buildPolyscopeGuiCustom();
    ImGui::End();

    ImGui::Begin("Polyscope Structure Info", nullptr, ImGuiWindowFlags_None);
    polyscope::buildStructureGuiCustom();
    ImGui::End();

    ImGui::Begin("Polyscope Picking Viewer", nullptr, ImGuiWindowFlags_None);
    polyscope::buildPickGuiCustom();
    ImGui::End();

    return true;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE