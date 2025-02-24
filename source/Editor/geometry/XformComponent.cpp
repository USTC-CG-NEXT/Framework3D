#include "GCore/Components/XformComponent.h"

#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/basisCurves.h>

#include "pxr/base/gf/rotation.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
GeometryComponentHandle XformComponent::copy(Geometry* operand) const
{
    using namespace pxr;
    auto ret = std::make_shared<XformComponent>(attached_operand);
    ret->attached_operand = operand;

    ret->rotation = rotation;
    ret->translation = translation;
    ret->scale = scale;

    return ret;
}

std::string XformComponent::to_string() const
{
    return std::string("XformComponent");
}

pxr::GfMatrix4d XformComponent::get_transform() const
{
    assert(translation.size() == rotation.size());
    pxr::GfMatrix4d final_transform;
    final_transform.SetIdentity();
    for (int i = 0; i < translation.size(); ++i) {
        pxr::GfMatrix4d t;
        t.SetTranslate(translation[i]);
        pxr::GfMatrix4d s;
        s.SetScale(scale[i]);
        pxr::GfMatrix4d r_x;
        r_x.SetRotate(pxr::GfRotation{ { 1, 0, 0 }, rotation[i][0] });
        pxr::GfMatrix4d r_y;
        r_y.SetRotate(pxr::GfRotation{ { 0, 1, 0 }, rotation[i][1] });
        pxr::GfMatrix4d r_z;
        r_z.SetRotate(pxr::GfRotation{ { 0, 0, 1 }, rotation[i][2] });
        auto transform = r_x * r_y * r_z * s * t;
        final_transform = final_transform * transform;
    }
    return final_transform;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
