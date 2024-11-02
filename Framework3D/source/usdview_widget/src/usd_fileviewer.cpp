#define IMGUI_DEFINE_MATH_OPERATORS

#include "widgets/usdtree/usd_fileviewer.h"

#include <iostream>

#include "imgui.h"
#include "imgui_internal.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/tf/ostreamMethods.h"
#include "pxr/base/vt/typeHeaders.h"
#include "pxr/base/vt/visitValue.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/primRange.h"
#include "pxr/usd/usd/property.h"
#include "stage/stage.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
void UsdFileViewer::ShowFileTree()
{
    auto root = stage->get_usd_stage()->GetPseudoRoot();
    ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit |
                            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                            ImGuiTableFlags_Resizable;
    if (ImGui::BeginTable("stage_table", 2, flags)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
        DrawChild(root);

        ImGui::EndTable();
    }
}

void UsdFileViewer::ShowPrimInfo()
{
    using namespace pxr;
    ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit |
                            ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                            ImGuiTableFlags_Resizable;
    if (ImGui::BeginTable("table", 3, flags)) {
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(
            "Property Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableHeadersRow();
        UsdPrim prim = stage->get_usd_stage()->GetPrimAtPath(selected);
        if (prim) {
            auto properties = prim.GetAttributes();

            for (auto&& attr : properties) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("A");
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(attr.GetName().GetText());

                ImGui::TableSetColumnIndex(2);
                VtValue v;
                attr.Get(&v);
                if (v.IsArrayValued()) {
                    std::string displayString;
                    auto formatArray = [&](auto array) {
                        size_t arraySize = array.size();
                        size_t displayCount = 3;
                        for (size_t i = 0;
                             i < std::min(displayCount, arraySize);
                             ++i) {
                            displayString += TfStringify(array[i]) + ", \n";
                        }
                        if (arraySize > 2 * displayCount) {
                            displayString += "... \n";
                        }
                        for (size_t i = std::max(
                                 displayCount, arraySize - displayCount);
                             i < arraySize;
                             ++i) {
                            displayString += TfStringify(array[i]) + ", \n";
                        }
                        if (!displayString.empty()) {
                            displayString.pop_back();
                            displayString.pop_back();
                            displayString.pop_back();
                        }
                    };
                    if (v.IsHolding<VtArray<double>>()) {
                        formatArray(v.Get<VtArray<double>>());
                    }
                    else if (v.IsHolding<VtArray<float>>()) {
                        formatArray(v.Get<VtArray<float>>());
                    }
                    else if (v.IsHolding<VtArray<GfMatrix4d>>()) {
                        formatArray(v.Get<VtArray<GfMatrix4d>>());
                    }
                    else if (v.IsHolding<VtArray<GfMatrix4f>>()) {
                        formatArray(v.Get<VtArray<GfMatrix4f>>());
                    }
                    else if (v.IsHolding<VtArray<GfVec2d>>()) {
                        formatArray(v.Get<VtArray<GfVec2d>>());
                    }
                    else if (v.IsHolding<VtArray<GfVec2f>>()) {
                        formatArray(v.Get<VtArray<GfVec2f>>());
                    }
                    else if (v.IsHolding<VtArray<GfVec3d>>()) {
                        formatArray(v.Get<VtArray<GfVec3d>>());
                    }
                    else if (v.IsHolding<VtArray<GfVec3f>>()) {
                        formatArray(v.Get<VtArray<GfVec3f>>());
                    }
                    else if (v.IsHolding<VtArray<GfVec4d>>()) {
                        formatArray(v.Get<VtArray<GfVec4d>>());
                    }
                    else if (v.IsHolding<VtArray<GfVec4f>>()) {
                        formatArray(v.Get<VtArray<GfVec4f>>());
                    }
                    else {
                        displayString = "Unsupported array type";
                    }
                    ImGui::TextUnformatted(displayString.c_str());
                }
                else {
                    auto s = VtVisitValue(
                        v, [](auto&& v) { return TfStringify(v); });
                    ImGui::TextUnformatted(s.c_str());
                }
            }
        }
        ImGui::EndTable();
    }
}

void UsdFileViewer::show_right_click_menu()
{
    if (ImGui::BeginPopupContextWindow("Prim Operation")) {
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Mesh")) {
                stage->create_mesh(selected);
            }
            if (ImGui::MenuItem("Cylinder")) {
                stage->create_cylinder(selected);
            }
            if (ImGui::MenuItem("Sphere")) {
                stage->create_sphere(selected);
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Edit")) {
            stage->create_editor_at_path(selected);
        }

        if (ImGui::MenuItem("Delete")) {
            stage->remove_prim(selected);
        }
        ImGui::EndPopup();
    }
}

void UsdFileViewer::DrawChild(const pxr::UsdPrim& prim)
{
    auto flags = ImGuiTreeNodeFlags_DefaultOpen |
                 ImGuiTreeNodeFlags_SpanAvailWidth |
                 ImGuiTreeNodeFlags_OpenOnArrow;

    bool is_leaf = prim.GetChildren().empty();
    if (is_leaf) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet |
                 ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    if (prim.GetPath() == selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    bool open = ImGui::TreeNodeEx(prim.GetName().GetText(), flags);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        selected = prim.GetPath();
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        selected = prim.GetPath();
        ImGui::OpenPopup("Prim Operation");
    }

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(prim.GetTypeName().GetText());

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        selected = prim.GetPath();
    }

    if (!is_leaf) {
        if (open) {
            for (const pxr::UsdPrim& child : prim.GetChildren()) {
                DrawChild(child);
            }

            ImGui::TreePop();
        }
    }

    if (prim.GetPath() == selected) {
        show_right_click_menu();
    }
}

bool UsdFileViewer::BuildUI()
{
    ImGui::Begin("Stage Viewer", nullptr, ImGuiWindowFlags_None);
    ShowFileTree();
    ImGui::End();

    ImGui::Begin("Prim Info", nullptr, ImGuiWindowFlags_None);
    ShowPrimInfo();
    ImGui::End();

    return true;
}

UsdFileViewer::UsdFileViewer(Stage* stage) : stage(stage)
{
}

UsdFileViewer::~UsdFileViewer()
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
