#pragma once
#include "RHI/ResourceManager/resource_allocator.hpp"
#include "pxr/base/vt/array.h"
#include "pxr/base/vt/hash.h"
#include "pxr/usd/sdf/path.h"

namespace USTC_CG {
class Hd_USTC_CG_RenderInstanceCollection;
class LensSystem;
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
        pxr::TfHashMap<pxr::SdfPath, Hd_USTC_CG_Material*, pxr::TfHash>*
            materials,
        nvrhi::IDevice* nvrhi_device)
        : cameras(cameras),
          lights(lights),
          materials(materials),
          nvrhi_device(nvrhi_device),
          shader_factory(&resource_allocator)
    {
        shader_factory.set_search_path(RENDERER_SHADER_DIR);
        resource_allocator.device = nvrhi_device;
        resource_allocator.shader_factory = &shader_factory;
    }

    RenderGlobalPayload(const RenderGlobalPayload& rhs)
        : cameras(rhs.cameras),
          lights(rhs.lights),
          materials(rhs.materials),
          nvrhi_device(rhs.nvrhi_device),
          shader_factory(&resource_allocator)
    {
        shader_factory.set_search_path(RENDERER_SHADER_DIR);
        resource_allocator.device = nvrhi_device;
        resource_allocator.shader_factory = &shader_factory;
    }

    RenderGlobalPayload& operator=(const RenderGlobalPayload& rhs)
    {
        cameras = rhs.cameras;
        lights = rhs.lights;
        materials = rhs.materials;
        nvrhi_device = rhs.nvrhi_device;
        shader_factory = ShaderFactory(&resource_allocator);
        shader_factory.set_search_path(RENDERER_SHADER_DIR);
        resource_allocator.device = nvrhi_device;
        resource_allocator.shader_factory = &shader_factory;
        return *this;
    }

    ResourceAllocator resource_allocator;
    ShaderFactory shader_factory;
    nvrhi::IDevice* nvrhi_device;
    Hd_USTC_CG_RenderInstanceCollection* InstanceCollection;
    bool reset_accumulation = false;

    auto& get_cameras() const
    {
        return *cameras;
    }

    auto& get_lights() const
    {
        return *lights;
    }

    auto& get_materials() const
    {
        return *materials;
    }

    LensSystem* lens_system;

   private:
    pxr::VtArray<Hd_USTC_CG_Camera*>* cameras;
    pxr::VtArray<Hd_USTC_CG_Light*>* lights;
    pxr::TfHashMap<pxr::SdfPath, Hd_USTC_CG_Material*, pxr::TfHash>* materials;
};

}  // namespace USTC_CG
