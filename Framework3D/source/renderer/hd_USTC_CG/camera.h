#pragma once
#include "config.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/imaging/hd/camera.h"
#include "renderDelegate.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace pxr;

class Hd_USTC_CG_Camera : public HdCamera {
   public:
    void Sync(
        HdSceneDelegate* sceneDelegate,
        HdRenderParam* renderParam,
        HdDirtyBits* dirtyBits) override;

    void update(const HdRenderPassStateSharedPtr& renderPassState) const;

    GfMatrix4f projMatrix;
    GfMatrix4f inverseProjMatrix;
    GfMatrix4f viewMatrix;
    GfMatrix4f inverseViewMatrix;
    GfRect2i dataWindow;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE