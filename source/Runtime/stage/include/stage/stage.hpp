#pragma once
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdLux/sphereLight.h>
#include <pxr/usd/usdSkel/skeletonQuery.h>

#include "pxr/usd/usdGeom/cube.h"
#include "pxr/usd/usdGeom/cylinder.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdGeom/xformCache.h"
#include "pxr/usd/usdLux/diskLight.h"
#include "pxr/usd/usdLux/distantLight.h"
#include "pxr/usd/usdLux/domeLight.h"
#include "pxr/usd/usdLux/rectLight.h"
#include "stage/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace animation {
class WithDynamicLogicPrim;
}

class STAGE_API Stage {
   public:
    Stage();
    ~Stage();
    // Add a new initializer for custom stage file
    Stage(const std::string& stage_path);

    bool should_simulate() const
    {
        return render_time_code >= current_time_code;
    }

    void tick(float ellapsed_time);
    void finish_tick();

    pxr::UsdTimeCode get_current_time();
    void set_current_time(pxr::UsdTimeCode time);

    pxr::UsdTimeCode get_render_time();
    void set_render_time(pxr::UsdTimeCode time);

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

    pxr::UsdLuxRectLight create_rect_light(
        const pxr::SdfPath& path = pxr::SdfPath::EmptyPath()) const;

    pxr::UsdLuxDistantLight create_distant_light(
        const pxr::SdfPath& path = pxr::SdfPath::EmptyPath()) const;

    pxr::UsdLuxDiskLight create_disk_light(
        const pxr::SdfPath& path = pxr::SdfPath::EmptyPath()) const;

    pxr::UsdLuxDomeLight create_dome_light(
        const pxr::SdfPath& path = pxr::SdfPath::EmptyPath()) const;

    pxr::UsdLuxSphereLight create_sphere_light(const pxr::SdfPath& path) const
    {
        return create_prim<pxr::UsdLuxSphereLight>(path, "sphere_light");
    }

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
    const std::string& GetStagePath()
    {
        return m_stage_path;
    }

   private:
    std::string m_stage_path;
    pxr::UsdStageRefPtr stage;
    pxr::SdfPath create_editor_pending_path;
    pxr::UsdTimeCode current_time_code = pxr::UsdTimeCode(0.0f);
    pxr::UsdTimeCode render_time_code = pxr::UsdTimeCode(0.0f);
    template<typename T>
    T create_prim(const pxr::SdfPath& path, const std::string& baseName) const;

    std::unordered_map<
        pxr::SdfPath,
        animation::WithDynamicLogicPrim,
        pxr::SdfPath::Hash>
        animatable_prims;
};

STAGE_API std::unique_ptr<Stage> create_global_stage();
STAGE_API std::unique_ptr<Stage> create_custom_global_stage(
    const std::string& filename);
USTC_CG_NAMESPACE_CLOSE_SCOPE
