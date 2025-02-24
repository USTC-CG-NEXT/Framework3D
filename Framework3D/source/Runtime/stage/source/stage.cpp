#include "stage/stage.hpp"

#include <pxr/pxr.h>
#include <pxr/usd/usd/payloads.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>

#include "animation.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
#define SAVE_ALL_THE_TIME 0

Stage::Stage()
{
    // if stage.usda exists, load it
    stage = pxr::UsdStage::Open("../../Assets/stage.usdc");
    if (stage) {
        return;
    }

    stage = pxr::UsdStage::CreateNew("../../Assets/stage.usdc");
    stage->SetMetadata(pxr::UsdGeomTokens->metersPerUnit, 1.0);
    stage->SetMetadata(pxr::UsdGeomTokens->upAxis, pxr::TfToken("Z"));
}

Stage::~Stage()
{
    remove_prim(pxr::SdfPath("/scratch_buffer"));
    stage->Save();
    animatable_prims.clear();
}

void Stage::tick(float ellapsed_time)
{
    auto current = current_time_code.GetValue();
    current += ellapsed_time;
    current_time_code = pxr::UsdTimeCode(current);

    // for each prim, if it is animatable, update it
    for (auto&& prim : stage->Traverse()) {
        if (animation::WithDynamicLogicPrim::is_animatable(prim)) {
            if (animatable_prims.find(prim.GetPath()) ==
                animatable_prims.end()) {
                animatable_prims[prim.GetPath()] =
                    std::move(animation::WithDynamicLogicPrim(prim));
            }

            animatable_prims[prim.GetPath()].update(ellapsed_time);
        }
    }
}

void Stage::finish_tick()
{
}

pxr::UsdTimeCode Stage::get_current_time()
{
    return current_time_code;
}

void Stage::set_current_time(pxr::UsdTimeCode time)
{
    current_time_code = time;
}

template<typename T>
T Stage::create_prim(const pxr::SdfPath& path, const std::string& baseName)
    const
{
    int id = 0;
    while (stage->GetPrimAtPath(
        path.AppendPath(pxr::SdfPath(baseName + "_" + std::to_string(id))))) {
        id++;
    }
    auto a = T::Define(
        stage,
        path.AppendPath(pxr::SdfPath(baseName + "_" + std::to_string(id))));
#if SAVE_ALL_THE_TIME
    stage->Save();
#endif
    return a;
}

pxr::UsdPrim Stage::add_prim(const pxr::SdfPath& path)
{
    return stage->DefinePrim(path);
}

pxr::UsdGeomSphere Stage::create_sphere(const pxr::SdfPath& path) const
{
    return create_prim<pxr::UsdGeomSphere>(path, "sphere");
}

pxr::UsdGeomCylinder Stage::create_cylinder(const pxr::SdfPath& path) const
{
    return create_prim<pxr::UsdGeomCylinder>(path, "cylinder");
}

pxr::UsdGeomCube Stage::create_cube(const pxr::SdfPath& path) const
{
    return create_prim<pxr::UsdGeomCube>(path, "cube");
}

pxr::UsdGeomXform Stage::create_xform(const pxr::SdfPath& path) const
{
    return create_prim<pxr::UsdGeomXform>(path, "xform");
}

pxr::UsdGeomMesh Stage::create_mesh(const pxr::SdfPath& path) const
{
    return create_prim<pxr::UsdGeomMesh>(path, "mesh");
}

void Stage::remove_prim(const pxr::SdfPath& path)
{
    if (animatable_prims.find(path) != animatable_prims.end()) {
        animatable_prims.erase(path);
    }
    stage->RemovePrim(path);  // This operation is in fact not recommended! In
                              // Omniverse applications, they set the prim to
                              // invisible instead of removing it.

#if SAVE_ALL_THE_TIME
    stage->Save();
#endif
}

std::string Stage::stage_content() const
{
    std::string str;
    stage->GetRootLayer()->ExportToString(&str);
    return str;
}

pxr::UsdStageRefPtr Stage::get_usd_stage() const
{
    return stage;
}

void Stage::create_editor_at_path(const pxr::SdfPath& sdf_path)
{
    create_editor_pending_path = sdf_path;
}

bool Stage::consume_editor_creation(pxr::SdfPath& json_path, bool fully_consume)
{
    if (create_editor_pending_path.IsEmpty()) {
        return false;
    }

    json_path = create_editor_pending_path;
    if (fully_consume) {
        create_editor_pending_path = pxr::SdfPath::EmptyPath();
    }
    return true;
}

void Stage::save_string_to_usd(
    const pxr::SdfPath& path,
    const std::string& data)
{
    auto prim = stage->GetPrimAtPath(path);
    if (!prim) {
        return;
    }

    auto attr = prim.CreateAttribute(
        pxr::TfToken("node_json"), pxr::SdfValueTypeNames->String);
    attr.Set(data);
#if SAVE_ALL_THE_TIME
    stage->Save();
#endif
}

std::string Stage::load_string_from_usd(const pxr::SdfPath& path)
{
    auto prim = stage->GetPrimAtPath(path);
    if (!prim) {
        return "";
    }

    auto attr = prim.GetAttribute(pxr::TfToken("node_json"));
    if (!attr) {
        return "";
    }

    std::string data;
    attr.Get(&data);
    return data;
}

void Stage::import_usd(
    const std::string& path_string,
    const pxr::SdfPath& sdf_path)
{
    auto prim = stage->GetPrimAtPath(sdf_path);
    if (!prim) {
        return;
    }

    // bring the usd file into the stage with payload

    auto paylaods = prim.GetPayloads();
    paylaods.AddPayload(pxr::SdfPayload(path_string));
#if SAVE_ALL_THE_TIME
    stage->Save();
#endif
}

std::unique_ptr<Stage> create_global_stage()
{
    return std::make_unique<Stage>();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
