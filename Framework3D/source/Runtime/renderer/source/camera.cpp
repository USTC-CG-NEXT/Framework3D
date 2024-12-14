#include "camera.h"
#include "pxr/imaging/hd/camera.h"

#include "config.h"
#include "renderDelegate.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/rect2i.h"
#include "pxr/imaging/cameraUtil/framing.h"
#include "pxr/imaging/hd/renderPassState.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace pxr;
void Hd_USTC_CG_Camera::Sync(
    HdSceneDelegate* sceneDelegate,
    HdRenderParam* renderParam,
    HdDirtyBits* dirtyBits)
{
    HdCamera::Sync(sceneDelegate, renderParam, dirtyBits);

    projMatrix = GfMatrix4f(this->ComputeProjectionMatrix());
    inverseProjMatrix = projMatrix.GetInverse();
     
    inverseViewMatrix = GfMatrix4f(GetTransform());
    viewMatrix = inverseViewMatrix.GetInverse();
}

static GfRect2i _GetDataWindow(
    const HdRenderPassStateSharedPtr& renderPassState)
{
    const CameraUtilFraming& framing = renderPassState->GetFraming();
    if (framing.IsValid()) {
        return framing.dataWindow;
    }
    else {
        const GfVec4f vp = renderPassState->GetViewport();
        return GfRect2i(GfVec2i(0), int(vp[2]), int(vp[3]));
    }
}

void Hd_USTC_CG_Camera::update(
    const HdRenderPassStateSharedPtr& renderPassState) const
{
    dataWindow = _GetDataWindow(renderPassState);

    projMatrix = GfMatrix4f(renderPassState->GetProjectionMatrix());
    inverseProjMatrix = projMatrix.GetInverse();
    viewMatrix = GfMatrix4f(renderPassState->GetWorldToViewMatrix());
    inverseViewMatrix = viewMatrix.GetInverse();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
