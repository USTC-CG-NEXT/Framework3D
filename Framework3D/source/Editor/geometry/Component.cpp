#include "stage/stage.hpp"

#include "GCore/Components.h"
#include "global_stage.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

GeometryComponent::~GeometryComponent()
{
    g_stage->remove_prim(scratch_buffer_path);
}

GeometryComponent::GeometryComponent(Geometry* attached_operand)
    : attached_operand(attached_operand)
{
    scratch_buffer_path = pxr::SdfPath(
        "/scratch_buffer/component_" +
        std::to_string(reinterpret_cast<long long>(this)));
}

Geometry* GeometryComponent::get_attached_operand() const
{
    return attached_operand;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE