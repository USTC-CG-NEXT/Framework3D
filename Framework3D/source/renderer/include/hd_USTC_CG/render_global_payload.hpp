#pragma once
#include "RHI/ResourceManager/resource_allocator.hpp"
#include "pxr/base/vt/array.h"
#include "pxr/base/vt/hash.h"
#include "pxr/usd/sdf/path.h"

namespace USTC_CG {
class Hd_USTC_CG_Light;
class Hd_USTC_CG_Camera;
class Hd_USTC_CG_Mesh;
class Hd_USTC_CG_Material;
}  // namespace USTC_CG

namespace USTC_CG {

struct RenderGlobalPayload {
    RenderGlobalPayload()
    {
    }
    RenderGlobalPayload(
        pxr::VtArray<Hd_USTC_CG_Camera*>* cameras,
        pxr::VtArray<Hd_USTC_CG_Light*>* lights,
        pxr::VtArray<Hd_USTC_CG_Mesh*>* meshes,
        pxr::TfHashMap<pxr::SdfPath, Hd_USTC_CG_Material*, pxr::TfHash>* materials,
        nvrhi::IDevice* nvrhi_device)
        : cameras(cameras),
          lights(lights),
          meshes(meshes),
          materials(materials),
          nvrhi_device(nvrhi_device)
    {
        resource_allocator.device = nvrhi_device;
    }

    ResourceAllocator resource_allocator;
    nvrhi::IDevice* nvrhi_device;

    auto& get_cameras() const
    {
        return *cameras;
    }

    auto& get_lights() const
    {
        return *lights;
    }

    auto& get_meshes() const
    {
        return *meshes;
    }

    auto& get_materials() const
    {
        return *materials;
    }

   private:
    pxr::VtArray<Hd_USTC_CG_Camera*>* cameras;
    pxr::VtArray<Hd_USTC_CG_Light*>* lights;
    pxr::VtArray<Hd_USTC_CG_Mesh*>* meshes;
    pxr::TfHashMap<pxr::SdfPath, Hd_USTC_CG_Material*, pxr::TfHash>* materials;
};

}  // namespace USTC_CG
