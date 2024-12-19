#include "material.h"

#include "api.h"
#include "pxr/imaging/hd/changeTracker.h"
#include "pxr/imaging/hd/sceneDelegate.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
Hd_USTC_CG_Material::Hd_USTC_CG_Material(SdfPath const& id) : HdMaterial(id)
{
}

void Hd_USTC_CG_Material::Sync(
    HdSceneDelegate* sceneDelegate,
    HdRenderParam* renderParam,
    HdDirtyBits* dirtyBits)
{
    //VtValue material = sceneDelegate->GetMaterialResource(GetId());
    //HdMaterialNetworkMap networkMap = material.Get<HdMaterialNetworkMap>();

    //if (networkMap.map.empty()) {
    //    return;
    //}
    //for (const auto& entry : networkMap.map) {
    //    std::cout << "Token: " << entry.first << std::endl;
    //    const HdMaterialNetwork& network = entry.second;
    //    for (const auto& node : network.nodes) {
    //        std::cout << "Node ID: " << node.path << std::endl;
    //        for (const auto& param : node.parameters) {
    //            std::cout << "Param: " << param.first << " = " << param.second << std::endl;
    //        }
    //    }
    //}

    //for (const auto& terminal : networkMap.terminals) {
    //    std::cout << "Terminal: " << terminal << std::endl;
    //}

    *dirtyBits = HdChangeTracker::Clean;
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