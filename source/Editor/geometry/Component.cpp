#include "GCore/Components.h"
#include "global_stage.hpp"
#include "stage/stage.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

GeometryComponent::~GeometryComponent()
{
#if USE_USD_SCRATCH_BUFFER
    g_stage->remove_prim(scratch_buffer_path);
#endif
}

GeometryComponent::GeometryComponent(Geometry* attached_operand)
    : attached_operand(attached_operand)
{
#if USE_USD_SCRATCH_BUFFER
    scratch_buffer_path = pxr::SdfPath(
        "/scratch_buffer/component_" +
        std::to_string(reinterpret_cast<long long>(this)));
#endif
}

Geometry* GeometryComponent::get_attached_operand() const
{
    return attached_operand;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE