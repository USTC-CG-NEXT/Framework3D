#include "material.h"

#include <pxr/imaging/hd/material.h>
#include <pxr/imaging/hd/materialNetwork2Interface.h>
#include <pxr/imaging/hdMtlx/hdMtlx.h>
#include <pxr/usdImaging/usdImaging/tokens.h>

#include "MaterialX/SlangShaderGenerator.h"
#include "MaterialXCore/Document.h"
#include "MaterialXFormat/Util.h"
#include "MaterialXGenShader/Shader.h"
#include "MaterialXGenShader/Util.h"
#include "api.h"
#include "pxr/imaging/hd/changeTracker.h"
#include "pxr/imaging/hd/sceneDelegate.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace mx = MaterialX;

MaterialX::GenContextPtr Hd_USTC_CG_Material::shader_gen_context_ =
    std::make_shared<mx::GenContext>(mx::SlangShaderGenerator::create());
MaterialX::DocumentPtr Hd_USTC_CG_Material::libraries = mx::createDocument();

std::once_flag Hd_USTC_CG_Material::shader_gen_initialized_;

Hd_USTC_CG_Material::Hd_USTC_CG_Material(SdfPath const& id) : HdMaterial(id)
{
    std::call_once(shader_gen_initialized_, []() {
        mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
        loadLibraries({ "libraries" }, searchPath, libraries);
        mx::loadLibraries(
            { "usd/hd_USTC_CG/resources/libraries" }, searchPath, libraries);
        searchPath.append(mx::FileSearchPath("usd/hd_USTC_CG/resources"));
        shader_gen_context_->registerSourceCodeSearchPath(searchPath);
    });
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

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (mtlx)

    // Hydra MaterialX Node Types
    (ND_standard_surface_surfaceshader)(ND_UsdPreviewSurface_surfaceshader)(ND_displacement_float)(ND_displacement_vector3)(ND_image_vector2)(ND_image_vector3)(ND_image_vector4)
    // For supporting Usd texturing nodes
    (wrapS)(wrapT)(repeat)(periodic)(ND_UsdUVTexture)(ND_dot_vector2)(ND_UsdPrimvarReader_vector2)(UsdPrimvarReader_float2)(UsdUVTexture)(UsdVerticalFlip)(varname)(st));

static TfToken _FixSingleType(TfToken const& nodeType)
{
    if (nodeType == UsdImagingTokens->UsdPreviewSurface) {
        return _tokens->ND_UsdPreviewSurface_surfaceshader;
    }
    else if (nodeType == UsdImagingTokens->UsdUVTexture) {
        return _tokens->ND_UsdUVTexture;
    }
    else if (nodeType == UsdImagingTokens->UsdPrimvarReader_float2) {
        return _tokens->ND_UsdPrimvarReader_vector2;
    }

    else {
        return TfToken("ND_" + nodeType.GetString());
    }
}

static void _FixNodeTypes(HdMaterialNetwork2Interface* netInterface)
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
                nodeType = _FixSingleType(nodeType);
            }
            netInterface->SetNodeType(nodeName, nodeType);
        }
    }
}

static void _FixNodeValues(HdMaterialNetwork2Interface* netInterface)
{
    // Fix textures wrap mode from repeat to periodic, because MaterialX does
    // not support repeat mode.
    const TfTokenVector nodeNames = netInterface->GetNodeNames();

    for (TfToken const& nodeName : nodeNames) {
        TfToken nodeType = netInterface->GetNodeType(nodeName);
        if (nodeType == _tokens->ND_UsdUVTexture) {
            VtValue wrapS =
                netInterface->GetNodeParameterValue(nodeName, _tokens->wrapS);
            VtValue wrapT =
                netInterface->GetNodeParameterValue(nodeName, _tokens->wrapT);
            if (wrapS.IsHolding<TfToken>() && wrapT.IsHolding<TfToken>()) {
                TfToken wrapSValue = wrapS.Get<TfToken>();
                TfToken wrapTValue = wrapT.Get<TfToken>();
                if (wrapSValue == _tokens->repeat) {
                    netInterface->SetNodeParameterValue(
                        nodeName, _tokens->wrapS, VtValue(_tokens->periodic));
                }
                if (wrapTValue == _tokens->repeat) {
                    netInterface->SetNodeParameterValue(
                        nodeName, _tokens->wrapT, VtValue(_tokens->periodic));
                }
            }
        }

        std::cout << "node name: " << nodeName.GetString()
                  << " node type: " << nodeType.GetString() << std::endl;
    }
}

void Hd_USTC_CG_Material::Sync(
    HdSceneDelegate* sceneDelegate,
    HdRenderParam* renderParam,
    HdDirtyBits* dirtyBits)
{
    VtValue material = sceneDelegate->GetMaterialResource(GetId());
    HdMaterialNetworkMap networkMap = material.Get<HdMaterialNetworkMap>();

    bool isVolume;
    HdMaterialNetwork2 hdNetwork =
        HdConvertToHdMaterialNetwork2(networkMap, &isVolume);

    auto materialPath = GetId();

    HdMaterialNetwork2Interface netInterface(materialPath, &hdNetwork);
    _FixNodeTypes(&netInterface);
    _FixNodeValues(&netInterface);

    const TfToken& terminalNodeName = HdMaterialTerminalTokens->surface;
    SdfPath surfTerminalPath;

    HdMaterialNode2 const* surfTerminal =
        _GetTerminalNode(hdNetwork, terminalNodeName, &surfTerminalPath);

    std::cout << surfTerminal->nodeTypeId.GetString() << std::endl;
    std::cout << surfTerminalPath.GetString() << std::endl;

    for (const auto& node : hdNetwork.nodes) {
        std::cout << node.first.GetString() << std::endl;
        std::cout << node.second.nodeTypeId.GetString() << std::endl;
    }

    if (surfTerminal) {
        HdMtlxTexturePrimvarData hdMtlxData;
        MaterialX::DocumentPtr mtlx_document =
            HdMtlxCreateMtlxDocumentFromHdNetwork(
                hdNetwork,
                *surfTerminal,
                surfTerminalPath,
                materialPath,
                libraries,
                &hdMtlxData);

        assert(mtlx_document);

        using namespace mx;
        auto materials = mtlx_document->getMaterialNodes();

        auto shaders =
            mtlx_document->getNodesOfType(SURFACE_SHADER_TYPE_STRING);

        std::cout << "Material Document: " << materials[0]->asString()
                  << std::endl;

        std::cout << "Shader: " << shaders[0]->asString() << std::endl;

        auto renderable = mx::findRenderableElements(mtlx_document);
        auto element = renderable[0];
        const std::string elementName(element->getNamePath());

        ShaderGenerator& shader_generator_ =
            shader_gen_context_->getShaderGenerator();
        auto shader = shader_generator_.generate(
            elementName, element, *shader_gen_context_);

        auto source_code = shader->getSourceCode();

        std::cout << "Generated Shader: " << source_code << std::endl;
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