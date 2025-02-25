// #undef _MSC_VER

#include "GCore/Components/PointsComponent.h"

#include "GCore/GOP.h"
#include "global_stage.hpp"
#include "stage/stage.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
PointsComponent::PointsComponent(Geometry* attached_operand)
    : GeometryComponent(attached_operand)
{
#if USE_USD_SCRATCH_BUFFER
    scratch_buffer_path = pxr::SdfPath(
        "/scratch_buffer/points_component_" +
        std::to_string(reinterpret_cast<long long>(this)));
    points = pxr::UsdGeomPoints::Define(
        g_stage->get_usd_stage(), scratch_buffer_path);
    pxr::UsdGeomImageable(points).MakeInvisible();
#endif
}

std::string PointsComponent::to_string() const
{
    std::ostringstream out;
    // Loop over the vertices and print the data
    out << "Points component. "
        << "Vertices count " << get_vertices().size()
        << ". Face vertices count "
        << ".";
    return out.str();
}

GeometryComponentHandle PointsComponent::copy(Geometry* operand) const
{
    auto ret = std::make_shared<PointsComponent>(operand);
#if USE_USD_SCRATCH_BUFFER
    copy_prim(points.GetPrim(), ret->points.GetPrim());
    pxr::UsdGeomImageable(points).MakeInvisible();
#else
    ret->set_vertices(this->get_vertices());
    ret->set_display_color(this->get_display_color());
    ret->set_width(this->get_width());
#endif

    return ret;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
