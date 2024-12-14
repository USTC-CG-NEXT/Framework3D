#include "material.h"

#include "api.h"
#include "pxr/imaging/hd/changeTracker.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
Hd_USTC_CG_Material::Hd_USTC_CG_Material(SdfPath const& id) : HdMaterial(id)
{
}

void Hd_USTC_CG_Material::Sync(
    HdSceneDelegate* sceneDelegate,
    HdRenderParam* renderParam,
    HdDirtyBits* dirtyBits)
{
}

HdDirtyBits Hd_USTC_CG_Material::GetInitialDirtyBitsMask() const
{
    return HdChangeTracker::AllDirty;
}

void Hd_USTC_CG_Material::Finalize(HdRenderParam* renderParam)
{
    HdMaterial::Finalize(renderParam);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE