#include "stage/stage.hpp"

#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>

USTC_CG_NAMESPACE_OPEN_SCOPE

Stage::Stage()
{
    stage = pxr::UsdStage::CreateNew("stage.usda");
}

template<typename T>
T Stage::create_prim(const pxr::SdfPath& path, const std::string& baseName)
    const
{
    int id = 0;
    while (stage->GetPrimAtPath(path.AppendPath(
        pxr::SdfPath("/" + baseName + "_" + std::to_string(id))))) {
        id++;
    }
    return T::Define(
        stage,
        path.AppendPath(
            pxr::SdfPath("/" + baseName + "_" + std::to_string(id))));
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
    stage->RemovePrim(path);
}

std::string Stage::stage_content() const
{
    std::string str;
    stage->GetRootLayer()->ExportToString(&str);
    return str;
}

std::unique_ptr<Stage> create_globale_stage()
{
    return std::make_unique<Stage>();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
