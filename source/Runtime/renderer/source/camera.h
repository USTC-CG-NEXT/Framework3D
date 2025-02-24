#pragma once
#include "config.h"
#include "api.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/rect2i.h"
#include "pxr/imaging/hd/camera.h"
#include "pxr/imaging/hdx/renderSetupTask.h"
#include "pxr/pxr.h"
#include "pxr/usd/sdf/path.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace pxr;

class Hd_USTC_CG_Camera : public HdCamera {
   public:
    explicit Hd_USTC_CG_Camera(SdfPath const& id) : HdCamera(id)
    {
    }

    void Sync(
        HdSceneDelegate* sceneDelegate,
        HdRenderParam* renderParam,
        HdDirtyBits* dirtyBits) override;

    void update(const HdRenderPassStateSharedPtr& renderPassState) const;

    mutable GfMatrix4f projMatrix;
    mutable GfMatrix4f inverseProjMatrix;
    mutable GfMatrix4f viewMatrix;
    mutable GfMatrix4f inverseViewMatrix;
    mutable GfRect2i dataWindow;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE