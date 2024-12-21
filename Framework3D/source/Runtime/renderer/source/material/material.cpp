#include "material.h"

#include <pxr/imaging/hd/material.h>
#include <pxr/imaging/hd/materialNetwork2Interface.h>
#include <pxr/imaging/hdMtlx/hdMtlx.h>
#include <pxr/usdImaging/usdImaging/tokens.h>

#include "MaterialXCore/Document.h"
#include "api.h"
#include "pxr/imaging/hd/changeTracker.h"
#include "pxr/imaging/hd/sceneDelegate.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
Hd_USTC_CG_Material::Hd_USTC_CG_Material(SdfPath const& id) : HdMaterial(id)
{
}

static HdMaterialNode2 const* _GetTerminalNode(
    HdMaterialNetwork2 const& network,
    TfToken const& terminalName,
    SdfPath* terminalNodePath)
{
    // Get the Surface or Volume Terminal
    auto const& terminalConnIt = network.terminals.find(terminalName);
    if (terminalConnIt == network.terminals.end()) {
        return nullptr;
    }
    HdMaterialConnection2 const& connection = terminalConnIt->second;
    SdfPath const& terminalPath = connection.upstreamNode;
    auto const& terminalIt = network.nodes.find(terminalPath);
    *terminalNodePath = terminalPath;
    return &terminalIt->second;
}

namespace mx = MaterialX;

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (mtlx)

    // Hydra MaterialX Node Types
    (ND_standard_surface_surfaceshader)(ND_UsdPreviewSurface_surfaceshader)(ND_displacement_float)(ND_displacement_vector3)(ND_image_vector2)(ND_image_vector3)(ND_image_vector4)
    // For supporting Usd texturing nodes
    (ND_UsdUVTexture)(ND_dot_vector2)(ND_UsdPrimvarReader_vector2)(UsdPrimvarReader_float2)(UsdUVTexture)(UsdVerticalFlip));

static void _FixNodeNames(HdMaterialNetworkInterface* netInterface)
{
    const TfTokenVector nodeNames = netInterface->GetNodeNames();
    for (TfToken const& nodeName : nodeNames) {
        TfToken nodeType = netInterface->GetNodeType(nodeName);
        std::cout << "node name: " << nodeName.GetString()
                  << " node type: " << nodeType.GetString() << std::endl;

        if (TfStringStartsWith(nodeType.GetText(), "Usd")) {
            if (nodeType == _tokens->UsdPrimvarReader_float2) {
                nodeType = _tokens->ND_UsdPrimvarReader_vector2;
            }
            else if (nodeType == _tokens->UsdVerticalFlip) {
                nodeType = _tokens->ND_dot_vector2;  // pass through node
            }
            else {
                nodeType = TfToken("ND_" + nodeType.GetString());
            }
            netInterface->SetNodeType(nodeName, nodeType);
        }
    }
}

void Hd_USTC_CG_Material::Sync(
    HdSceneDelegate* sceneDelegate,
    HdRenderParam* renderParam,
    HdDirtyBits* dirtyBits)
{
    *dirtyBits = HdChangeTracker::Clean;

    return;
    VtValue material = sceneDelegate->GetMaterialResource(GetId());
    HdMaterialNetworkMap networkMap = material.Get<HdMaterialNetworkMap>();

    bool isVolume;
    HdMaterialNetwork2 hdNetwork =
        HdConvertToHdMaterialNetwork2(networkMap, &isVolume);

    auto materialPath = GetId();

    HdMaterialNetwork2Interface netInterface(materialPath, &hdNetwork);
    _FixNodeNames(&netInterface);

    const TfToken& terminalNodeName = HdMaterialTerminalTokens->surface;
    SdfPath surfTerminalPath;

    const mx::DocumentPtr& libraries = HdMtlxStdLibraries();
    HdMaterialNode2 const* surfTerminal =
        _GetTerminalNode(hdNetwork, terminalNodeName, &surfTerminalPath);

    std::cout << surfTerminal->nodeTypeId.GetString() << std::endl;

    if (surfTerminal) {
        HdMtlxTexturePrimvarData hdMtlxData;
        MaterialX::DocumentPtr mtlx_document =
            HdMtlxCreateMtlxDocumentFromHdMaterialNetworkInterface(
                &netInterface,
                terminalNodeName,
                netInterface.GetNodeInputConnectionNames(terminalNodeName),
                libraries,
                &hdMtlxData);

        assert(mtlx_document);

        std::cout << "MaterialX Document: " << mtlx_document->asString()
                  << std::endl;
    }

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