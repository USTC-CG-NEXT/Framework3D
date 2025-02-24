#pragma once
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>

#include "pxr/usd/usdGeom/cube.h"
#include "pxr/usd/usdGeom/cylinder.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "pxr/usd/usdGeom/xform.h"
#include "stage/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace animation {
class WithDynamicLogicPrim;
}

class STAGE_API Stage {
   public:
    Stage();
    ~Stage();

    void tick(float ellapsed_time);
    void finish_tick();

    pxr::UsdTimeCode get_current_time();
    void set_current_time(pxr::UsdTimeCode time);

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

    void create_editor_at_path(const pxr::SdfPath& sdf_path);
    bool consume_editor_creation(
        pxr::SdfPath& json_path,
        bool fully_consume = true);
    void save_string_to_usd(const pxr::SdfPath& path, const std::string& data);
    std::string load_string_from_usd(const pxr::SdfPath& path);
    void import_usd(
        const std::string& path_string,
        const pxr::SdfPath& sdf_path);

   private:
    pxr::UsdStageRefPtr stage;
    pxr::SdfPath create_editor_pending_path;
    pxr::UsdTimeCode current_time_code = pxr::UsdTimeCode::Default();
    template<typename T>
    T create_prim(const pxr::SdfPath& path, const std::string& baseName) const;

    pxr::TfHashMap<
        pxr::SdfPath,
        animation::WithDynamicLogicPrim,
        pxr::SdfPath::Hash>
        animatable_prims;
};

STAGE_API std::unique_ptr<Stage> create_global_stage();

USTC_CG_NAMESPACE_CLOSE_SCOPE