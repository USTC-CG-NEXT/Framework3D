#define IMGUI_DEFINE_MATH_OPERATORS

#include "widgets/usdtree/usd_fileviewer.h"

#include <future>
#include <iostream>
#include <vector>
#include <filesystem>
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
#include "pxr/usd/usdGeom/xformOp.h"
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
            auto attributes = prim.GetAttributes();
            std::vector<std::future<std::string>> futures;

            for (auto&& attr : attributes) {
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

            auto relations = prim.GetRelationships();
            std::vector<std::future<std::string>> relation_futures;
            for (auto&& relation : relations) {
                relation_futures.push_back(
                    std::async(std::launch::async, [&relation]() {
                        std::string displayString;
                        SdfPathVector relation_targets;
                        relation.GetTargets(&relation_targets);
                        for (auto&& target : relation_targets) {
                            displayString += target.GetString() + ",\n";
                        }
                        if (!displayString.empty()) {
                            displayString.pop_back();
                            displayString.pop_back();
                        }
                        return displayString;
                    }));
            }
            auto displayRow = [](const char* type,
                                 const std::string& name,
                                 const std::string& value) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(type);
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(name.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(value.c_str());
            };

            for (size_t i = 0; i < attributes.size(); ++i) {
                displayRow(
                    "A", attributes[i].GetName().GetString(), futures[i].get());
            }

            for (size_t i = 0; i < relations.size(); ++i) {
                displayRow(
                    "R",
                    relations[i].GetName().GetString(),
                    relation_futures[i].get());
            }
        }
        ImGui::EndTable();
    }
}

void UsdFileViewer::EditValue()
{
    using namespace pxr;
    UsdPrim prim = stage->get_usd_stage()->GetPrimAtPath(selected);
    if (prim) {

        auto xformable = UsdGeomXformable::Get(stage->get_usd_stage(), selected);
        if (xformable){
//            log::warning("XFORMABLE_GET! -> %s", prim.GetName().GetText());
            bool rst_stack;
            auto xform_op = xformable.GetOrderedXformOps(&rst_stack);
            if (xform_op.size() == 0){ // no trans
                auto trans = xformable.AddTranslateOp(UsdGeomXformOp::PrecisionFloat);
                auto rot = xformable.AddRotateXYZOp(UsdGeomXformOp::PrecisionFloat);
                auto sca = xformable.AddScaleOp(UsdGeomXformOp::PrecisionFloat);
                trans.Set(GfVec3f{0,0,0});
                rot.Set(GfVec3f{0,0,0});
                sca.Set(GfVec3f{1,1,1});
                xformable.SetXformOpOrder({trans, rot,sca});
            }
            else if (xform_op.size() == 3 && xform_op[0].GetOpType() == UsdGeomXformOp::TypeTranslate
                     && xform_op[1].GetOpType() == UsdGeomXformOp::TypeRotateXYZ
                     && xform_op[2].GetOpType() == UsdGeomXformOp::TypeScale){

                auto trans = xform_op[0];
                auto rot = xform_op[1];
                auto sca = xform_op[2];

                GfVec3f trans_vec; trans.Get(&trans_vec);
                GfVec3f rot_vec; rot.Get(&rot_vec);
                GfVec3f sca_vec; sca.Get(&sca_vec);

                const auto FloatSliderAndInputSetterForArray = [](auto& vec, int pos, float slow, float fast, float sliderminimum, float slidermaximum, std::string name){
                  float value = vec[pos];
                  if (ImGui::SliderFloat((name + "(Slider)").c_str(), &value, sliderminimum, slidermaximum)){vec[pos] = value;}
                  if (ImGui::InputFloat((name + "(Input)").c_str(), &value, slow, fast)){vec[pos] = value;}

                };

                FloatSliderAndInputSetterForArray(trans_vec, 0, 0.01, 1, -10, 10, "Tran X");
                FloatSliderAndInputSetterForArray(trans_vec, 1, 0.01, 1, -10, 10, "Tran Y");
                FloatSliderAndInputSetterForArray(trans_vec, 2, 0.01, 1, -10, 10, "Tran Z");
                FloatSliderAndInputSetterForArray(rot_vec, 0, 0.1, 1, -180, 180, "Rot X");
                FloatSliderAndInputSetterForArray(rot_vec, 1, 0.1, 1, -180, 180, "Rot Y");
                FloatSliderAndInputSetterForArray(rot_vec, 2, 0.1, 1, -180, 180, "Rot Z");
                FloatSliderAndInputSetterForArray(sca_vec, 0, 0.01, 0.5, 0, 10, "Sca X");
                FloatSliderAndInputSetterForArray(sca_vec, 1, 0.01, 0.5, 0, 10, "Sca Y");
                FloatSliderAndInputSetterForArray(sca_vec, 2, 0.01, 0.5, 0, 10, "Sca Z");

                trans.Set(trans_vec);
                rot.Set(rot_vec);
                sca.Set(sca_vec);

                xformable.SetXformOpOrder({trans, rot, sca});
            }
            // else: do nothing
        }




        auto attributes = prim.GetAttributes();
        for (auto&& attr : attributes) {
            VtValue v;
            attr.Get(&v);
            std::string label =
                attr.GetName().GetString() + "##" + attr.GetName().GetString();

            /// print the label for dbg
//            log::warning("%s -> %s [%s]",prim.GetName().GetText(),label.c_str(), attr.GetTypeName().GetCPPTypeName().c_str());


            if (v.IsHolding<double>()) {
                double value = v.Get<double>();
                double min_double = 0;
                double max_double = 100;
                double step_slow = 0.1;
                double step_fast = 1;
                if (ImGui::SliderScalar(
                        label.c_str(),
                        ImGuiDataType_Double,
                        &value,
                        &min_double,
                        &max_double)) {
                    attr.Set(value);
                }

                if (ImGui::InputScalar(
                        (label+" (Input)").c_str(),
                    ImGuiDataType_Double,
                        &value, &step_slow, &step_fast
                        )){
                    value = std::max(min_double, value);
                    attr.Set(value);
                }

            }
            else if (v.IsHolding<float>()) {
                float value = v.Get<float>();
                float min_float = 0;
                float max_float = 100;
                float step_slow = 0.1;
                float step_fast = 1;
                if (ImGui::SliderFloat(
                        label.c_str(), &value, min_float, max_float)) {
                    attr.Set(value);
                }
                 if (ImGui::InputScalar(
                        (label+" (Input)").c_str(),
                    ImGuiDataType_Float,
                        &value, &step_slow, &step_fast
                        )){
                    value = std::max(min_float, value);
                    attr.Set(value);
                }

            }
            else if (v.IsHolding<int>()) {
                int value = v.Get<int>();
                int min_int = -10;
                int max_int = 10;
                if (ImGui::SliderInt(label.c_str(), &value, min_int, max_int)) {
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<unsigned int>()) {
                unsigned int value = v.Get<unsigned int>();
                unsigned int min_uint = 0;
                unsigned int max_uint = 10;
                if (ImGui::SliderScalar(
                        label.c_str(),
                        ImGuiDataType_U32,
                        &value,
                        &min_uint,
                        &max_uint)) {
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<int64_t>()) {
                int64_t value = v.Get<int64_t>();
                int64_t min_int64 = -10;
                int64_t max_int64 = 10;
                if (ImGui::SliderScalar(
                        label.c_str(),
                        ImGuiDataType_S64,
                        &value,
                        &min_int64,
                        &max_int64)) {
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<GfVec2f>()) {
                GfVec2f value = v.Get<GfVec2f>();
                if (ImGui::SliderFloat2(
                        label.c_str(), value.data(), 0.0f, 1.0f)) {
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<GfVec3f>()) {
                GfVec3f value = v.Get<GfVec3f>();
                if (ImGui::SliderFloat3(
                        label.c_str(), value.data(), 0.0f, 1.0f)) {
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<GfVec4f>()) {
                GfVec4f value = v.Get<GfVec4f>();
                if (ImGui::SliderFloat4(
                        label.c_str(), value.data(), 0.0f, 1.0f)) {
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<GfVec2i>()) {
                GfVec2i value = v.Get<GfVec2i>();
                if (ImGui::SliderInt2(label.c_str(), value.data(), -10, 10)) {
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<GfVec3i>()) {
                GfVec3i value = v.Get<GfVec3i>();
                if (ImGui::SliderInt3(label.c_str(), value.data(), -10, 10)) {
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<GfVec4i>()) {
                GfVec4i value = v.Get<GfVec4i>();
                if (ImGui::SliderInt4(label.c_str(), value.data(), -10, 10)) {
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<GfVec2d>()) {
                GfVec2d value = v.Get<GfVec2d>();
                float tmp[2] = {value[0], value[1]};
                if (ImGui::SliderFloat2(
                        label.c_str(), tmp, 0.0f, 1.0f)) {
                    value[0]=tmp[0];
                    value[1] = tmp[1];
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<GfVec3d>()) {
                GfVec3d value = v.Get<GfVec3d>();
                float tmp[3] = {value[0], value[1],value[2]};

                if (ImGui::SliderFloat3(
                        label.c_str(), tmp, 0.0f, 1.0f)) {
                    value[0]=tmp[0];
                    value[1] = tmp[1];
                    value[2] = tmp[2];
                    attr.Set(value);
                }
            }
            else if (v.IsHolding<GfVec4d>()) {
                GfVec4d value = v.Get<GfVec4d>();
                float tmp[4] = {value[0], value[1],value[2], value[3]};

                if (ImGui::SliderFloat4(
                        label.c_str(), tmp, 0.0f, 1.0f)) {
                    value[0]=tmp[0];
                    value[1] = tmp[1];
                    value[2] = tmp[2];
                    value[3] = tmp[3];
                    attr.Set(value);
                }
            }

//            if (v.IsHolding<VtArray<TfToken>>()){
//                std::cout << "1\n";
//            }
//
//            if (label=="xformOpOrder##xformOpOrder") {
//                std::cout << v.IsEmpty() << "\n";
//                log::warning("[%s]", label.c_str());
//
//            }


        }
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

int UsdFileViewer::delete_pass_id = 0;

void UsdFileViewer::remove_prim_logic()
{
    if (delete_pass_id == 3) {
        stage->remove_prim(to_delete);
    }

    if (delete_pass_id == 2) {
        stage->add_prim(to_delete);
    }

    if (delete_pass_id == 1) {
        stage->remove_prim(to_delete);
    }

    if (delete_pass_id > 0) {
        delete_pass_id--;
    }
}

void UsdFileViewer::show_right_click_menu()
{
    if (ImGui::BeginPopupContextWindow("Prim Operation")) {
        if (ImGui::BeginMenu("Create Geometry")) {
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

        if (ImGui::BeginMenu("Create Light")) {
            if (ImGui::MenuItem("Dome Light")) {
                is_config_dome = true;
                selected_for_dome = selected;
            }
            if (ImGui::MenuItem("Disk Light")) {
                stage->create_disk_light(selected);
            }
            if (ImGui::MenuItem("Distant Light")) {
                stage->create_distant_light(selected);
            }
            if (ImGui::MenuItem("Rect Light")) {
                stage->create_rect_light(selected);
            }
            if (ImGui::MenuItem("Sphere Light")) {
                stage->create_sphere_light(selected);
            }
            ImGui::EndMenu();
        }



        if (selected != pxr::SdfPath("/")) {
            if (ImGui::MenuItem("Import...")) {
                is_selecting_file = true;
                selecting_file_base = selected;
            }
            if (ImGui::MenuItem("Edit")) {
                stage->create_editor_at_path(selected);
            }

            if (ImGui::MenuItem("Delete")) {
                to_delete = selected;
                delete_pass_id = 3;
            }
        }

        ImGui::EndPopup();
    }
}

void UsdFileViewer::conf_dome(){
    if (!is_config_dome) return ;
    ImGui::Begin("Config dome");

    ImGui::InputTextWithHint("Envmap path", "Input the relative path to the env map", buf, 255);
    std::string relative_path(buf);
    namespace fs = std::filesystem;

    bool invalid = false;
    ImGui::Text(selected_for_dome.GetText());
    // check the path
    if (!fs::is_regular_file(relative_path)) {ImGui::Text("This is not a FILE!"); invalid=true;}
    if (!invalid && !fs::exists(relative_path)){ImGui::Text("File not exists!"); invalid=true;}
    if (!invalid && (!relative_path.ends_with(".exr") && !relative_path.ends_with(".hdr"))){ImGui::Text("Only exr/hdr file is supported!");}

    if (!invalid){
        // the path is valid
        ImGui::Text("Valid path!");
        if (ImGui::Button("OK")){
            auto dome = stage->create_dome_light(selected_for_dome);
            auto asset_path = pxr::SdfAssetPath(relative_path.c_str());
             dome.CreateTextureFileAttr().Set(pxr::VtValue(asset_path));
            is_config_dome=false;
        }
    }

    if(ImGui::Button("Exit")){
        is_config_dome=false;
    }
    ImGui::End();
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

    ImGui::Begin("Edit Value", nullptr, ImGuiWindowFlags_None);
    EditValue();
    ImGui::End();
    remove_prim_logic();
    conf_dome();

    return true;
}

UsdFileViewer::UsdFileViewer(Stage* stage) : stage(stage)
{
}

UsdFileViewer::~UsdFileViewer()
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
