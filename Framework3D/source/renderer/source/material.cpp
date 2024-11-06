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

void Hd_USTC_CG_Material::TryLoadTexture(
    const char* str,
    InputDescriptor& descriptor,
    HdMaterialNode2& usd_preview_surface)
{
}

void Hd_USTC_CG_Material::TryLoadParameter(
    const char* str,
    InputDescriptor& descriptor,
    HdMaterialNode2& usd_preview_surface)
{
}

void Hd_USTC_CG_Material::TryCreateGLTexture(InputDescriptor& descriptor)
{
}

HdMaterialNode2 Hd_USTC_CG_Material::get_input_connection(
    HdMaterialNetwork2 surfaceNetwork,
    std::map<TfToken, std::vector<HdMaterialConnection2>>::value_type&
        input_connection)
{
    return HdMaterialNode2();
}

void Hd_USTC_CG_Material::DestroyTexture(InputDescriptor& input_descriptor)
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE