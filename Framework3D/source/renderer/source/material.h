#pragma once
#include "api.h"
#include "Logger/Logger.h"
#include "map.h"
#include "pxr/imaging/garch/glApi.h"
#include "pxr/imaging/hd/material.h"
#include "pxr/imaging/hio/image.h"

namespace pxr {
class Hio_OpenEXRImage;

}

USTC_CG_NAMESPACE_OPEN_SCOPE
class Shader;
using namespace pxr;

class Hio_StbImage;
class HD_USTC_CG_API Hd_USTC_CG_Material : public HdMaterial {
   public:
    struct InputDescriptor {
        HioImageSharedPtr image = nullptr;

        TfToken wrapS;
        TfToken wrapT;

        TfToken uv_primvar_name;

        VtValue value;

        GLuint glTexture = 0;
        TfToken input_name;
    };

    explicit Hd_USTC_CG_Material(SdfPath const& id);

    void Sync(HdSceneDelegate* sceneDelegate, HdRenderParam* renderParam, HdDirtyBits* dirtyBits)
        override;

    HdDirtyBits GetInitialDirtyBitsMask() const override;

    TfToken requireTexcoordName();

    void Finalize(HdRenderParam* renderParam) override;

    InputDescriptor diffuseColor;
    InputDescriptor specularColor;
    InputDescriptor emissiveColor;
    InputDescriptor displacement;
    InputDescriptor opacity;
    InputDescriptor opacityThreshold;
    InputDescriptor roughness;
    InputDescriptor metallic;
    InputDescriptor clearcoat;
    InputDescriptor clearcoatRoughness;
    InputDescriptor occlusion;
    InputDescriptor normal;
    InputDescriptor ior;

   private:
    HdMaterialNetwork2 surfaceNetwork;

    void TryLoadTexture(
        const char* str,
        InputDescriptor& descriptor,
        HdMaterialNode2& usd_preview_surface);
    void TryLoadParameter(
        const char* str,
        InputDescriptor& descriptor,
        HdMaterialNode2& usd_preview_surface);

    void TryCreateGLTexture(InputDescriptor& descriptor);

    HdMaterialNode2 get_input_connection(
        HdMaterialNetwork2 surfaceNetwork,
        std::map<TfToken, std::vector<HdMaterialConnection2>>::value_type& input_connection);
    void DestroyTexture(InputDescriptor& input_descriptor);
};



USTC_CG_NAMESPACE_CLOSE_SCOPE