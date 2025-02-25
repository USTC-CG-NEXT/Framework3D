#include "GCore/Components/CurveComponent.h"

#include "global_stage.hpp"
#include "stage/stage.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
std::string CurveComponent::to_string() const
{
    std::ostringstream out;
    out << "Curve component. "
        << "Vertices count " << get_vertices().size()
        << ". Face vertices count " << ".";
    return out.str();
}

GeometryComponentHandle CurveComponent::copy(Geometry* operand) const
{
    auto ret = std::make_shared<CurveComponent>(operand);
#if USE_USD_SCRATCH_BUFFER
    copy_prim(curves.GetPrim(), ret->curves.GetPrim());
    pxr::UsdGeomImageable(curves).MakeInvisible();
#else
    ret->set_vertices(this->get_vertices());
    ret->set_display_color(this->get_display_color());
    ret->set_width(this->get_width());
    ret->set_vert_count(this->get_vert_count());
    ret->set_periodic(this->get_periodic());
    ret->set_curve_normals(this->get_curve_normals());
#endif
    return ret;
}

CurveComponent::CurveComponent(Geometry* attached_operand)
    : GeometryComponent(attached_operand)
{
#if USE_USD_SCRATCH_BUFFER
    scratch_buffer_path = pxr::SdfPath(
        "/scratch_buffer/curves_component_" +
        std::to_string(reinterpret_cast<long long>(this)));
    curves = pxr::UsdGeomBasisCurves::Define(
        g_stage->get_usd_stage(), scratch_buffer_path);
    pxr::UsdGeomImageable(curves).MakeInvisible();
#endif
}

USTC_CG_NAMESPACE_CLOSE_SCOPE