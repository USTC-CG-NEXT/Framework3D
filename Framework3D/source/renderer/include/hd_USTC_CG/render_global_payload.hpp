#pragma once
#include "pxr/usd/sdf/path.h"
#include "RHI/ResourceManager/resource_allocator.hpp"

namespace USTC_CG {
class Hd_USTC_CG_Light;
class Hd_USTC_CG_Camera;
class Hd_USTC_CG_Mesh;
class Hd_USTC_CG_Material;
}  // namespace USTC_CG

namespace USTC_CG {

struct RenderGlobalPayload {
    std::vector<Hd_USTC_CG_Camera*> cameras;
    std::vector<Hd_USTC_CG_Light*> lights;
    std::vector<Hd_USTC_CG_Mesh*> meshes;

    std::map<pxr::SdfPath, Hd_USTC_CG_Material*> materials;
    ResourceAllocator resource_allocator;
};

}  // namespace USTC_CG
