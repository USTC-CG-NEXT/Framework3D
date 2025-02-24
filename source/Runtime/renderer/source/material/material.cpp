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
#include "pxr/base/arch/fileSystem.h"
#include "pxr/base/arch/hash.h"
#include "pxr/base/arch/library.h"
#include "pxr/imaging/hd/changeTracker.h"
#include "pxr/imaging/hd/sceneDelegate.h"
#include "pxr/usd/ar/resolver.h"
#include "pxr/usd/sdr/registry.h"
#include "pxr/usd/sdr/shaderNode.h"

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
    (wrapS)(wrapT)(repeat)(periodic)(ND_UsdUVTexture)(ND_dot_vector2)(ND_UsdPrimvarReader_vector2)(UsdPrimvarReader_float2)(UsdUVTexture)(UsdVerticalFlip)(varname)(file)(filename)(black)(clamp)(uaddressmode)(vaddressmode)(ND_geompropvalue_vector2)(ND_separate2_vector2)(ND_floor_float)(ND_multiply_float)(ND_add_float)(ND_subtract_float)(ND_combine2_vector2)(separate2)(floor)(multiply)(add)(subtract)(combine2)(texcoord)(geomprop)(geompropvalue)(in)(in1)(in2)(out)(outx)(outy)(st)(vector2)((
        string_type,
        "string"))  // Color Space
    ((cs_raw, "raw"))((cs_auto, "auto"))((cs_srgb, "sRGB"))(
        (mtlx_srgb, "srgb_texture")));

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

static bool _FindGraphAndNodeByName(
    mx::DocumentPtr const& mxDoc,
    std::string const& mxNodeGraphName,
    std::string const& mxNodeName,
    mx::NodeGraphPtr* mxNodeGraph,
    mx::NodePtr* mxNode)
{
    // Graph names are uniquified with mxDoc->createValidChildName in hdMtlx,
    // so attempting to get the graph by the expected name may fail.
    // Go to some extra effort to find the graph that contains the named node.

    *mxNodeGraph = mxDoc->getNodeGraph(mxNodeGraphName);

    if (*mxNodeGraph) {
        *mxNode = (*mxNodeGraph)->getNode(mxNodeName);
    }
    if (!*mxNode) {
        std::vector<mx::NodeGraphPtr> graphs = mxDoc->getNodeGraphs();
        // first try last graph
        if (graphs.size()) {
            *mxNode = (*(graphs.rbegin()))->getNode(mxNodeName);
            if (*mxNode) {
                *mxNodeGraph = *graphs.rbegin();
            }
        }
        // Sometimes the above approach fails, so go looking
        // through all the graph nodes for the texture
        if (!*mxNode) {
            for (auto graph : graphs) {
                *mxNode = graph->getNode(mxNodeName);
                if (*mxNode) {
                    *mxNodeGraph = graph;
                    break;
                }
            }
        }
    }
    return (*mxNode != nullptr);
}
// Get the Hydra equivalent for the given MaterialX input value
static TfToken _GetHdWrapString(
    TfToken const& hdTextureNodeName,
    std::string const& mxInputValue)
{
    if (mxInputValue == "constant") {
        TF_WARN(
            "RtxHioImagePlugin: Texture '%s' has unsupported wrap mode "
            "'constant' using 'black' instead.",
            hdTextureNodeName.GetText());
        return _tokens->black;
    }
    if (mxInputValue == "clamp") {
        return _tokens->clamp;
    }
    if (mxInputValue == "mirror") {
        TF_WARN(
            "RtxHioImagePlugin: Texture '%s' has unsupported wrap mode "
            "'mirror' using 'repeat' instead.",
            hdTextureNodeName.GetText());
        return _tokens->repeat;
    }
    return _tokens->repeat;
}

static void _GetWrapModes(
    HdMaterialNetworkInterface* netInterface,
    TfToken const& hdTextureNodeName,
    TfToken* uWrap,
    TfToken* vWrap)
{
    // For <tiledimage> nodes want to always use "repeat"
    *uWrap = _tokens->repeat;
    *vWrap = _tokens->repeat;

    // For <image> nodes:
    VtValue vUAddrMode = netInterface->GetNodeParameterValue(
        hdTextureNodeName, _tokens->uaddressmode);
    if (!vUAddrMode.IsEmpty()) {
        *uWrap = _GetHdWrapString(
            hdTextureNodeName, vUAddrMode.UncheckedGet<std::string>());
    }
    VtValue vVAddrMode = netInterface->GetNodeParameterValue(
        hdTextureNodeName, _tokens->vaddressmode);
    if (!vVAddrMode.IsEmpty()) {
        *vWrap = _GetHdWrapString(
            hdTextureNodeName, vVAddrMode.UncheckedGet<std::string>());
    }
}

// Returns true is the given mtlxSdrNode requires primvar support for texture
// coordinates
static bool _NodeHasTextureCoordPrimvar(
    mx::DocumentPtr const& mxDoc,
    const SdrShaderNodeConstPtr mtlxSdrNode)
{
    // Custom nodes may have a <texcoord> or <geompropvalue> node as
    // a part of the defining nodegraph
    const mx::NodeDefPtr mxNodeDef =
        mxDoc->getNodeDef(mtlxSdrNode->GetIdentifier().GetString());
    mx::InterfaceElementPtr impl = mxNodeDef->getImplementation();
    if (impl && impl->isA<mx::NodeGraph>()) {
        const mx::NodeGraphPtr nodegraph = impl->asA<mx::NodeGraph>();
        // Return True if the defining nodegraph uses a texcoord node
        if (!nodegraph->getNodes(_tokens->texcoord).empty()) {
            return true;
        }
        // Or a geompropvalue node of type vector2, which we assume to be
        // for texture coordinates.
        auto geompropvalueNodes = nodegraph->getNodes(_tokens->geompropvalue);
        for (const mx::NodePtr& mxGeomPropNode : geompropvalueNodes) {
#if MATERIALX_MAJOR_VERSION == 1 && MATERIALX_MINOR_VERSION <= 38
            if (mxGeomPropNode->getType() == mx::Type::VECTOR2->getName()) {
#else
            if (mxGeomPropNode->getType() == mx::Type::VECTOR2.getName()) {
#endif
                return true;
            }
        }
    }
    return false;
}

static TfToken _GetColorSpace(
    HdMaterialNetworkInterface* netInterface,
#if PXR_VERSION >= 2402
    TfToken const& hdTextureNodeName,
    HdMaterialNetworkInterface::NodeParamData paramData)
#else
    TfToken const& hdTextureNodeName)
#endif
{
    const TfToken nodeType = netInterface->GetNodeType(hdTextureNodeName);
    if (nodeType == _tokens->ND_image_vector2 ||
        nodeType == _tokens->ND_image_vector3 ||
        nodeType == _tokens->ND_image_vector4) {
        // For images not used as color use "raw" (eg. normal maps)
        return _tokens->cs_raw;
    }
    else {
#if PXR_VERSION >= 2402
        if (paramData.colorSpace == _tokens->mtlx_srgb) {
            return _tokens->cs_srgb;
        }
        else {
            return _tokens->cs_auto;
        }
#else
        return _tokens->cs_auto;
#endif
    }
}
static void _UpdateTextureNodes(
    HdMaterialNetworkInterface* netInterface,
    std::set<SdfPath> const& hdTextureNodePaths,
    mx::DocumentPtr const& mxDoc)
{
    for (SdfPath const& texturePath : hdTextureNodePaths) {
        TfToken const& textureNodeName = texturePath.GetToken();
        std::string mxTextureNodeName = HdMtlxCreateNameFromPath(texturePath);
        const TfToken nodeType = netInterface->GetNodeType(textureNodeName);
        if (nodeType.IsEmpty()) {
            TF_WARN(
                "Connot find texture node '%s' in material network.",
                textureNodeName.GetText());
            continue;
        }
        // Get the filename parameter name,
        // MaterialX stdlib nodes use 'file' however, this could be different
        // for custom nodes that use textures.
        TfToken fileParamName = _tokens->file;
        const mx::NodeDefPtr nodeDef = mxDoc->getNodeDef(nodeType);
        if (nodeDef) {
            for (auto const& mxInput : nodeDef->getActiveInputs()) {
                if (mxInput->getType() == _tokens->filename) {
                    fileParamName = TfToken(mxInput->getName());
                }
            }
        }
#if PXR_VERSION >= 2402
        HdMaterialNetworkInterface::NodeParamData fileParamData =
            netInterface->GetNodeParameterData(textureNodeName, fileParamName);
        const VtValue vFile = fileParamData.value;
#else
        VtValue vFile =
            netInterface->GetNodeParameterValue(textureNodeName, fileParamName);
#endif
        if (vFile.IsEmpty()) {
            TF_WARN(
                "File path missing for texture node '%s'.",
                textureNodeName.GetText());
            continue;
        }

        std::string path;

        // Typically expect SdfAssetPath, but UsdUVTexture nodes may
        // have changed value to string due to MatfiltConvertPreviewMaterial
        // inserting rtxplugin call.
        if (vFile.IsHolding<SdfAssetPath>()) {
            path = vFile.Get<SdfAssetPath>().GetResolvedPath();
            if (path.empty()) {
                path = vFile.Get<SdfAssetPath>().GetAssetPath();
            }
        }
        else if (vFile.IsHolding<std::string>()) {
            path = vFile.Get<std::string>();
        }
        // Convert to posix path beause windows backslashes will get lost
        // before reaching the rtx plugin
        path = mx::FilePath(path).asString(mx::FilePath::FormatPosix);

        if (!path.empty()) {
            const std::string ext = ArGetResolver().GetExtension(path);

            mx::NodeGraphPtr mxNodeGraph;
            mx::NodePtr mxTextureNode;
            _FindGraphAndNodeByName(
                mxDoc,
                texturePath.GetParentPath().GetName(),
                mxTextureNodeName,
                &mxNodeGraph,
                &mxTextureNode);

            if (!mxTextureNode) {
                continue;
            }

            // Update texture nodes that use non-native texture formats
            // to read them via a Renderman texture plugin.
            bool needInvertT = false;
            if (TfStringStartsWith(path, "rtxplugin:")) {
                mxTextureNode->setInputValue(
                    _tokens->file.GetText(),       // name
                    path,                          // value
                    _tokens->filename.GetText());  // type
            }
            else if (!ext.empty() && ext != "tex") {
                // Update the input value to use the Renderman texture plugin
                const std::string pluginName =
                    std::string("RtxHioImage") + ARCH_LIBRARY_SUFFIX;

                TfToken uWrap, vWrap;
                _GetWrapModes(netInterface, textureNodeName, &uWrap, &vWrap);

#if PXR_VERSION >= 2402
                TfToken colorSpace = _GetColorSpace(
                    netInterface, textureNodeName, fileParamData);
#else
                TfToken colorSpace =
                    _GetColorSpace(netInterface, textureNodeName);
#endif

                std::string const& mxInputValue = TfStringPrintf(
                    "rtxplugin:%s?filename=%s&wrapS=%s&wrapT=%s&"
                    "sourceColorSpace=%s",
                    pluginName.c_str(),
                    path.c_str(),
                    uWrap.GetText(),
                    vWrap.GetText(),
                    colorSpace.GetText());

                // Update the MaterialX Texture Node with the new mxInputValue
                mxTextureNode->setInputValue(
                    fileParamName.GetText(),       // name
                    mxInputValue,                  // value
                    _tokens->filename.GetText());  // type
            }
            else {
                needInvertT = true;
                // For tex files, update value with resolved path, because prman
                // may not be able to find a usd relative path.
                mxTextureNode->setInputValue(
                    _tokens->file.GetText(),       // name
                    path,                          // value
                    _tokens->filename.GetText());  // type
            }

            // UsdUvTexture nodes and MtlxImage nodes have different
            // names for their texture coordinate connection.
            const TfToken texCoordToken = (nodeType == _tokens->ND_UsdUVTexture)
                                              ? _tokens->st
                                              : _tokens->texcoord;

            // If texcoord param isn't connected, make a default connection
            // to a mtlx geompropvalue node.
            mx::InputPtr texcoordInput = mxTextureNode->getInput(texCoordToken);
            if (!texcoordInput) {
                // Get the sdr node for the mxTexture node
                SdrRegistry& sdrRegistry = SdrRegistry::GetInstance();
                const SdrShaderNodeConstPtr sdrTextureNode =
                    sdrRegistry.GetShaderNodeByIdentifierAndType(
                        nodeType, _tokens->mtlx);

                // If the node does not already contain a texcoord primvar node
                // add one and connect it to the mxTextureNode
                // XXX If a custom node uses a texture but does not explicitly
                // use a texcoords or geomprop node for the texture coordinates
                // this will force a connection onto the custom node and the
                // material will likely not render.
                if (!_NodeHasTextureCoordPrimvar(mxDoc, sdrTextureNode)) {
                    // Get the primvarname from the sdrTextureNode metadata
                    auto metadata = sdrTextureNode->GetMetadata();
                    auto primvarName = metadata[SdrNodeMetadata->Primvars];

                    // Create a geompropvalue node for the texture coordinates
                    const std::string stNodeName =
                        textureNodeName.GetString() + "__texcoord";
                    mx::NodePtr geompropNode = mxNodeGraph->addNode(
                        _tokens->geompropvalue, stNodeName, _tokens->vector2);
                    geompropNode->setInputValue(
                        _tokens->geomprop, primvarName, _tokens->string_type);
                    geompropNode->setNodeDefString(
                        _tokens->ND_geompropvalue_vector2);

                    // Add the texcoord input and connect to the new node
                    texcoordInput = mxTextureNode->addInput(
                        _tokens->texcoord, _tokens->vector2);
                    texcoordInput->setConnectedNode(geompropNode);
                }
            }
            if (needInvertT) {
                // This inserts standard mtlx nodes to carry out the math
                // for udim aware invert of t; only want to flip
                // the fractional portion of the t value, like this:
                // 2*floor(t) + 1.0 - t
                texcoordInput = mxTextureNode->getInput(texCoordToken);
                if (texcoordInput) {
                    mx::NodePtr primvarNode = texcoordInput->getConnectedNode();
                    const std::string separateNodeName =
                        mxTextureNodeName + "__separate";
                    const std::string floorNodeName =
                        mxTextureNodeName + "__floor";
                    const std::string multiplyNodeName =
                        mxTextureNodeName + "__multiply";
                    const std::string addNodeName = mxTextureNodeName + "__add";
                    const std::string subtractNodeName =
                        mxTextureNodeName + "__subtract";
                    const std::string combineNodeName =
                        mxTextureNodeName + "__combine";

                    mx::NodePtr separateNode = mxNodeGraph->addNode(
                        _tokens->separate2, separateNodeName, _tokens->vector2);
                    separateNode->setNodeDefString(
                        _tokens->ND_separate2_vector2);

                    mx::NodePtr floorNode =
                        mxNodeGraph->addNode(_tokens->floor, floorNodeName);
                    floorNode->setNodeDefString(_tokens->ND_floor_float);

                    mx::NodePtr multiplyNode = mxNodeGraph->addNode(
                        _tokens->multiply, multiplyNodeName);
                    multiplyNode->setNodeDefString(_tokens->ND_multiply_float);

                    mx::NodePtr addNode =
                        mxNodeGraph->addNode(_tokens->add, addNodeName);
                    addNode->setNodeDefString(_tokens->ND_add_float);

                    mx::NodePtr subtractNode = mxNodeGraph->addNode(
                        _tokens->subtract, subtractNodeName);
                    subtractNode->setNodeDefString(_tokens->ND_subtract_float);

                    mx::NodePtr combineNode = mxNodeGraph->addNode(
                        _tokens->combine2, combineNodeName);
                    combineNode->setNodeDefString(_tokens->ND_combine2_vector2);

                    mx::InputPtr separateNode_inInput =
                        separateNode->addInput(_tokens->in, _tokens->vector2);
                    mx::OutputPtr separateNode_outxOutput =
                        separateNode->addOutput(_tokens->outx);
                    mx::OutputPtr separateNode_outyOutput =
                        separateNode->addOutput(_tokens->outy);
                    separateNode_inInput->setConnectedNode(primvarNode);

                    mx::InputPtr floorNode_inInput =
                        floorNode->addInput(_tokens->in);
                    mx::OutputPtr floorNode_outOutput =
                        floorNode->addOutput(_tokens->out);
                    floorNode_inInput->setConnectedNode(separateNode);
                    floorNode_inInput->setConnectedOutput(
                        separateNode_outyOutput);

                    mx::InputPtr multiplyNode_in1Input =
                        multiplyNode->addInput(_tokens->in1);
                    mx::OutputPtr multiplyNode_outOutput =
                        multiplyNode->addOutput(_tokens->out);
                    multiplyNode_in1Input->setConnectedNode(floorNode);
                    multiplyNode->setInputValue(_tokens->in2, 2);

                    mx::InputPtr addNode_in1Input =
                        addNode->addInput(_tokens->in1);
                    mx::OutputPtr addNode_outOutput =
                        addNode->addOutput(_tokens->out);
                    addNode_in1Input->setConnectedNode(multiplyNode);
                    addNode->setInputValue(_tokens->in2, 1);

                    mx::InputPtr subtractNode_in1Input =
                        subtractNode->addInput(_tokens->in1);
                    mx::InputPtr subtractNode_in2Input =
                        subtractNode->addInput(_tokens->in2);
                    mx::OutputPtr subtractNode_outOutput =
                        subtractNode->addOutput(_tokens->out);
                    subtractNode_in1Input->setConnectedNode(addNode);
                    subtractNode_in2Input->setConnectedNode(separateNode);
                    subtractNode_in2Input->setConnectedOutput(
                        separateNode_outyOutput);

                    mx::InputPtr combineNode_in1Input =
                        combineNode->addInput(_tokens->in1);
                    mx::InputPtr combineNode_in2Input =
                        combineNode->addInput(_tokens->in2);
                    mx::OutputPtr combineNode_outOutput =
                        combineNode->addOutput(_tokens->out, _tokens->vector2);
                    combineNode_in1Input->setConnectedNode(separateNode);
                    combineNode_in2Input->setConnectedNode(subtractNode);
                    texcoordInput->setConnectedNode(combineNode);
                }
            }
        }
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

        _UpdateTextureNodes(
            &netInterface, hdMtlxData.hdTextureNodes, mtlx_document);

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