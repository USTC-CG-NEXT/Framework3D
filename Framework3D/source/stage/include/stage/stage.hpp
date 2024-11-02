#pragma once
#include <pxr/usd/usd/stage.h>

#include "pxr/usd/usdGeom/cube.h"
#include "pxr/usd/usdGeom/cylinder.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "pxr/usd/usdGeom/xform.h"
#include "stage/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class STAGE_API Stage {
   public:
    Stage();
    ~Stage();

    pxr::UsdPrim add_prim(const pxr::SdfPath& path);

    pxr::UsdGeomSphere create_sphere(
        const pxr::SdfPath& path = pxr::SdfPath::EmptyPath()) const;
    pxr::UsdGeomCylinder create_cylinder(
        const pxr::SdfPath& path = pxr::SdfPath::EmptyPath()) const;
    pxr::UsdGeomCube create_cube(
        const pxr::SdfPath& path = pxr::SdfPath::EmptyPath()) const;
    pxr::UsdGeomXform create_xform(
        const pxr::SdfPath& path = pxr::SdfPath::EmptyPath()) const;
    pxr::UsdGeomMesh create_mesh(
        const pxr::SdfPath& path = pxr::SdfPath::EmptyPath()) const;

    void remove_prim(const pxr::SdfPath& path);

    [[nodiscard]] std::string stage_content() const;

    [[nodiscard]] pxr::UsdStageRefPtr get_usd_stage() const;

   private:
    pxr::UsdStageRefPtr stage;
    template<typename T>
    T create_prim(const pxr::SdfPath& path, const std::string& baseName) const;
};

STAGE_API std::unique_ptr<Stage> create_global_stage();

USTC_CG_NAMESPACE_CLOSE_SCOPE