#define IMGUI_DEFINE_MATH_OPERATORS

#include "widgets/usdtree/usd_fileviewer.h"

#include <future>
#include <iostream>
#include <vector>

#include "GUI/ImGuiFileDialog.h"
#include "Logger/Logger.h"
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
        DrawChild(root, true);

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
            std::vector<std::future<std::string>> futures;

            for (auto&& attr : properties) {
                futures.push_back(std::async(std::launch::async, [&attr]() {
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
                        else if (v.IsHolding<VtArray<int>>()) {
                            formatArray(v.Get<VtArray<int>>());
                        }
                        else if (v.IsHolding<VtArray<unsigned int>>()) {
                            formatArray(v.Get<VtArray<unsigned int>>());
                        }
                        else if (v.IsHolding<VtArray<int64_t>>()) {
                            formatArray(v.Get<VtArray<int64_t>>());
                        }
                        else if (v.IsHolding<VtArray<uint64_t>>()) {
                            formatArray(v.Get<VtArray<uint64_t>>());
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
                        else if (v.IsHolding<VtArray<GfVec2i>>()) {
                            formatArray(v.Get<VtArray<GfVec2i>>());
                        }
                        else if (v.IsHolding<VtArray<GfVec3d>>()) {
                            formatArray(v.Get<VtArray<GfVec3d>>());
                        }
                        else if (v.IsHolding<VtArray<GfVec3f>>()) {
                            formatArray(v.Get<VtArray<GfVec3f>>());
                        }
                        else if (v.IsHolding<VtArray<GfVec3i>>()) {
                            formatArray(v.Get<VtArray<GfVec3i>>());
                        }
                        else if (v.IsHolding<VtArray<GfVec4d>>()) {
                            formatArray(v.Get<VtArray<GfVec4d>>());
                        }
                        else if (v.IsHolding<VtArray<GfVec4f>>()) {
                            formatArray(v.Get<VtArray<GfVec4f>>());
                        }
                        else if (v.IsHolding<VtArray<GfVec4i>>()) {
                            formatArray(v.Get<VtArray<GfVec4i>>());
                        }
                        else {
                            displayString = "Unsupported array type";
                        }
                        return displayString;
                    }
                    else {
                        return VtVisitValue(
                            v, [](auto&& v) { return TfStringify(v); });
                    }
                }));
            }

            for (size_t i = 0; i < properties.size(); ++i) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("A");
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(properties[i].GetName().GetText());

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(futures[i].get().c_str());
            }
        }
        ImGui::EndTable();
    }
}

void UsdFileViewer::select_file()
{
    auto instance = IGFD::FileDialog::Instance();
    if (instance->Display("SelectFile")) {
        auto selected = instance->GetFilePathName();
        log::info(selected.c_str());

        is_selecting_file = false;

        stage->import_usd(selected, selecting_file_base);
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

        if (ImGui::MenuItem("Import...")) {
            is_selecting_file = true;
            selecting_file_base = selected;
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

void UsdFileViewer::DrawChild(const pxr::UsdPrim& prim, bool is_root)
{
    auto flags =
        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
    if (is_root) {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

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
    if (is_selecting_file) {
        select_file();
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
