// #define __GNUC__

#include "material.h"

#include "Logger/Logger.h"
#include "RHI/internal/map.h"
#include "pxr/imaging/hd/sceneDelegate.h"
#include "pxr/imaging/hio/image.h"
#include "pxr/usd/sdr/shaderNode.h"
#include "pxr/usd/usd/tokens.h"
#include "pxr/usdImaging/usdImaging/tokens.h"
#include "renderParam.h"
#include "texture.h"
#include "utils/sampling.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace pxr;

// Here for the cource purpose, we support a very limited set of forms of the
// material. Specifically, we support only UsdPreviewSurface, and each input can
// be either value, or a texture connected to a primvar reader.

HdMaterialNode2 Hd_USTC_CG_Material::get_input_connection(
    HdMaterialNetwork2 surfaceNetwork,
    std::map<TfToken, std::vector<HdMaterialConnection2>>::value_type&
        input_connection)
{
    HdMaterialNode2 upstream;
    assert(input_connection.second.size() == 1);
    upstream = surfaceNetwork.nodes[input_connection.second[0].upstreamNode];
    return upstream;
}

Hd_USTC_CG_Material::MaterialRecord Hd_USTC_CG_Material::SampleMaterialRecord(
    GfVec2f texcoord)
{
    MaterialRecord ret;
    if (diffuseColor.image) {
        auto val4 = diffuseColor.image->Evaluate(texcoord);
        ret.diffuseColor = { val4[0], val4[1], val4[2] };
    }
    else {
        ret.diffuseColor = diffuseColor.value.Get<GfVec3f>();
    }

    if (roughness.image) {
        auto val4 = roughness.image->Evaluate(texcoord);
        ret.roughness = val4[1];
    }
    else {
        ret.roughness = roughness.value.Get<float>();
    }

    if (ior.image) {
        auto val4 = ior.image->Evaluate(texcoord);
        ret.ior = val4[0];
    }
    else {
        ret.ior = ior.value.Get<float>();
    }

    if (metallic.image) {
        auto val4 = metallic.image->Evaluate(texcoord);
        ret.metallic = val4[2];
    }
    else {
        ret.metallic = metallic.value.Get<float>();
    }

    return ret;
}

void Hd_USTC_CG_Material::TryLoadTexture(
    const char* name,
    InputDescriptor& descriptor,
    HdMaterialNode2& usd_preview_surface)
{
    for (auto&& input_connection : usd_preview_surface.inputConnections) {
        if (input_connection.first == TfToken(name)) {
            log::info("Loading texture: " + input_connection.first.GetString());
            auto texture_node =
                get_input_connection(surfaceNetwork, input_connection);
            assert(texture_node.nodeTypeId == UsdImagingTokens->UsdUVTexture);

            auto assetPath =
                texture_node.parameters[TfToken("file")].Get<SdfAssetPath>();

            HioImage::SourceColorSpace colorSpace;

            if (texture_node.parameters[TfToken("sourceColorSpace")] ==
                TfToken("sRGB")) {
                colorSpace = HioImage::SRGB;
            }
            else {
                colorSpace = HioImage::Raw;
            }

            descriptor.image =
                std::make_unique<Texture2D>(assetPath, colorSpace);
            if (!descriptor.image->isValid()) {
                descriptor.image = nullptr;
            }
            descriptor.wrapS =
                texture_node.parameters[TfToken("wrapS")].Get<TfToken>();
            descriptor.wrapT =
                texture_node.parameters[TfToken("wrapT")].Get<TfToken>();

            HdMaterialNode2 st_read_node;
            for (auto&& st_read_connection : texture_node.inputConnections) {
                st_read_node =
                    get_input_connection(surfaceNetwork, st_read_connection);
            }

            assert(
                st_read_node.nodeTypeId ==
                UsdImagingTokens->UsdPrimvarReader_float2);
            descriptor.uv_primvar_name =
                st_read_node.parameters[TfToken("varname")].Get<TfToken>();
        }
    }
}

void Hd_USTC_CG_Material::TryLoadParameter(
    const char* name,
    InputDescriptor& descriptor,
    HdMaterialNode2& usd_preview_surface)
{
    for (auto&& parameter : usd_preview_surface.parameters) {
        if (parameter.first == name) {
            descriptor.value = parameter.second;
            log::info("Loading parameter: " + parameter.first.GetString());
        }
    }
}

#define INPUT_LIST                                                            \
    diffuseColor, specularColor, emissiveColor, displacement, opacity,        \
        opacityThreshold, roughness, metallic, clearcoat, clearcoatRoughness, \
        occlusion, normal, ior

#define TRY_LOAD(INPUT)                                 \
    TryLoadTexture(#INPUT, INPUT, usd_preview_surface); \
    TryLoadParameter(#INPUT, INPUT, usd_preview_surface);

#define NAME_IT(INPUT) INPUT.input_name = TfToken(#INPUT);

Hd_USTC_CG_Material::Hd_USTC_CG_Material(const SdfPath& id) : HdMaterial(id)
{
    log::info("Creating material " + id.GetString());
    diffuseColor.value = VtValue(GfVec3f(0.8f));
    roughness.value = VtValue(0.8f);

    metallic.value = VtValue(0.0f);
    normal.value = VtValue(GfVec3f(0.5, 0.5, 1.0));
    ior.value = VtValue(1.5f);

    MACRO_MAP(NAME_IT, INPUT_LIST);
}

void Hd_USTC_CG_Material::Sync(
    HdSceneDelegate* sceneDelegate,
    HdRenderParam* renderParam,
    HdDirtyBits* dirtyBits)
{
    static_cast<Hd_USTC_CG_RenderParam*>(renderParam)->AcquireSceneForEdit();

    VtValue vtMat = sceneDelegate->GetMaterialResource(GetId());
    if (vtMat.IsHolding<HdMaterialNetworkMap>()) {
        const HdMaterialNetworkMap& hdNetworkMap =
            vtMat.UncheckedGet<HdMaterialNetworkMap>();
        if (!hdNetworkMap.terminals.empty() && !hdNetworkMap.map.empty()) {
            log::info("Loaded a material");

            surfaceNetwork = HdConvertToHdMaterialNetwork2(hdNetworkMap);

            // Here we only support single output material.
            assert(surfaceNetwork.terminals.size() == 1);

            auto terminal =
                surfaceNetwork.terminals[HdMaterialTerminalTokens->surface];

            auto usd_preview_surface =
                surfaceNetwork.nodes[terminal.upstreamNode];
            assert(
                usd_preview_surface.nodeTypeId ==
                UsdImagingTokens->UsdPreviewSurface);

            MACRO_MAP(TRY_LOAD, INPUT_LIST)
        }
    }
    else {
        log::info("Not loaded a material");
    }
    *dirtyBits = Clean;
}

HdDirtyBits Hd_USTC_CG_Material::GetInitialDirtyBitsMask() const
{
    return AllDirty;
}

#define requireTexCoord(INPUT)              \
    if (!INPUT.uv_primvar_name.empty()) { \
        return INPUT.uv_primvar_name;       \
    }

std::string Hd_USTC_CG_Material::requireTexcoordName()
{
    MACRO_MAP(requireTexCoord, INPUT_LIST)
    return {};
}

void Hd_USTC_CG_Material::Finalize(HdRenderParam* renderParam)
{
    static_cast<Hd_USTC_CG_RenderParam*>(renderParam)->AcquireSceneForEdit();

    HdMaterial::Finalize(renderParam);
}

// TODO: (Optional) Modify this function to support more types of BRDF
Color Hd_USTC_CG_Material::Sample(
    const GfVec3f& wo,
    GfVec3f& wi,
    float& pdf,
    GfVec2f texcoord,
    const std::function<float()>& uniform_float)
{
    auto sample2D = GfVec2f{ uniform_float(), uniform_float() };

    // Given below are some of the variables you may need to guide your sampling
    // For the simplest ideal diffuse reflection, none of this is necessary
    auto record = SampleMaterialRecord(texcoord);
    auto roughness = std::clamp(record.roughness, 0.04f, 1.0f);
    auto ior = record.ior;
    auto metallic = record.metallic;
    GfVec3f diffuseColor = record.diffuseColor;


    wi = CosineWeightedDirection(sample2D, pdf);
    return Eval(wi, wo, texcoord);
}

// TODO: (Optional) Modify this function to support more types of BRDF
Color Hd_USTC_CG_Material::Eval(GfVec3f wi, GfVec3f wo, GfVec2f texcoord)
{
    auto record = SampleMaterialRecord(texcoord);

    GfVec3f diffuseColor = record.diffuseColor;

    // Given below are some of the variables you may need to calculate the brdf
    // For the simplest ideal diffuse reflection, none of this is necessary
    auto roughness = std::clamp(record.roughness, 0.04f, 1.0f);
    auto ior = record.ior;
    auto metallic = record.metallic;


    GfVec3f result = diffuseColor / M_PI;

    return result;
}

// TODO: (Optional) Modify this function to support MIS
// If you do not use multiple importance sampling,
// then this function is actually not very meaningful,
// because you have already got the corresponding PDF when you sample,
// and you do not need to call this function for evaluation
float Hd_USTC_CG_Material::Pdf(GfVec3f wi, GfVec3f wo, GfVec2f texcoord)
{
    return 0;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
