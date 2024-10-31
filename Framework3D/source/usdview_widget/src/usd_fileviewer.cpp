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

USTC_CG_NAMESPACE_OPEN_SCOPE

UsdFileViewer::UsdFileViewer()
{
}

void UsdFileViewer::set_stage(pxr::UsdStageRefPtr root_stage)
{
    stage = root_stage;
}

void UsdFileViewer::ShowFileTree()
{
    auto root = stage->GetPseudoRoot();
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
        UsdPrim prim = stage->GetPrimAtPath(selected);
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
                if (!v.IsArrayValued()) {
                    auto s = VtVisitValue(
                        v, [](auto&& v) { return TfStringify(v); });
                    ImGui::TextUnformatted(s.c_str());
                }
            }
        }
        ImGui::EndTable();
    }
}

pxr::SdfPath UsdFileViewer::emit_editor_info_path()
{
    auto temp = editor_info_path;
    editor_info_path = pxr::SdfPath::EmptyPath();
    return temp;
}

void UsdFileViewer::show_right_click_menu()
{
    // if (ImGui::BeginPopupContextWindow("Prim Operation")) {
    //     if (ImGui::BeginMenu("Create")) {
    //         if (ImGui::MenuItem("Mesh")) {
    //             GlobalUsdStage::CreateObject(selected, ObjectType::Mesh);
    //         }
    //         if (ImGui::MenuItem("Cylinder")) {
    //             GlobalUsdStage::CreateObject(selected, ObjectType::Cylinder);
    //         }
    //         if (ImGui::MenuItem("Sphere")) {
    //             GlobalUsdStage::CreateObject(selected, ObjectType::Sphere);
    //         }

    //        ImGui::EndMenu();
    //    }

    //    if (ImGui::MenuItem("Edit")) {
    //        editor_info_path = GlobalUsdStage::EditObject(selected);
    //    }

    //    if (ImGui::MenuItem("Delete")) {
    //        GlobalUsdStage::DeleteObject(selected);
    //    }
    //    ImGui::EndPopup();
    //}
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

UsdFileViewer::~UsdFileViewer()
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
