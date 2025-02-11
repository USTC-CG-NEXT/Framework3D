#pragma once
#include "Logger/Logger.h"
#include "MaterialX/SlangShaderGenerator.h"
#include "api.h"
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
    explicit Hd_USTC_CG_Material(SdfPath const& id);

    void Sync(
        HdSceneDelegate* sceneDelegate,
        HdRenderParam* renderParam,
        HdDirtyBits* dirtyBits) override;

    HdDirtyBits GetInitialDirtyBitsMask() const override;

    void Finalize(HdRenderParam* renderParam) override;

   private:
    HdMaterialNetwork2 surfaceNetwork;

    static MaterialX::GenContextPtr shader_gen_context_;
    static MaterialX::DocumentPtr libraries;

    static std::once_flag shader_gen_initialized_;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE