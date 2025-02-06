#pragma once

#include "GCore/Components.h"
#include "GCore/api.h"
#include "pxr/usd/usdGeom/xform.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
// Stores the chain of transformation

class GEOMETRY_API XformComponent : public GeometryComponent {
   public:
    GeometryComponentHandle copy(Geometry* operand) const override;
    std::string to_string() const override;

    explicit XformComponent(Geometry* attached_operand)
        : GeometryComponent(attached_operand)
    {
    }

    void apply_transform(const pxr::GfMatrix4d& transform) override
    {
    }

    pxr::GfMatrix4d get_transform() const;

    std::vector<pxr::GfVec3f> translation;
    std::vector<pxr::GfVec3f> scale;
    std::vector<pxr::GfVec3f> rotation;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
