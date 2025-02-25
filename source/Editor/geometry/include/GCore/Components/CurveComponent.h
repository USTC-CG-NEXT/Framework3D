#pragma once
#include <string>

#include "GCore/Components.h"
#include "GCore/GOP.h"
#include "pxr/usd/usdGeom/basisCurves.h"
#include "pxr/usd/usdGeom/curves.h"
#include "pxr/usd/usdGeom/xform.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct GEOMETRY_API CurveComponent : public GeometryComponent {
    explicit CurveComponent(Geometry* attached_operand);

    std::string to_string() const override;

    void apply_transform(const pxr::GfMatrix4d& transform) override
    {
        auto vertices = get_vertices();
        for (auto& vertex : vertices) {
            vertex = pxr::GfVec3f(transform.Transform(vertex));
        }
        set_vertices(vertices);
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_vertices() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<pxr::GfVec3f> vertices;
        if (curves.GetPointsAttr())
            curves.GetPointsAttr().Get(&vertices);
        return vertices;
#else
        return vertices;
#endif
    }

    void set_vertices(const pxr::VtArray<pxr::GfVec3f>& vertices)
    {
#if USE_USD_SCRATCH_BUFFER
        curves.CreatePointsAttr().Set(vertices);
#else
        this->vertices = vertices;
#endif
    }

    [[nodiscard]] pxr::VtArray<float> get_width() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<float> width;
        if (curves.GetWidthsAttr())
            curves.GetWidthsAttr().Get(&width);
        return width;
#else
        return width;
#endif
    }

    void set_width(const pxr::VtArray<float>& width)
    {
#if USE_USD_SCRATCH_BUFFER
        curves.CreateWidthsAttr().Set(width);
#else
        this->width = width;
#endif
    }

    [[nodiscard]] pxr::VtArray<int> get_vert_count() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<int> vert_count;
        if (curves.GetCurveVertexCountsAttr())
            curves.GetCurveVertexCountsAttr().Get(&vert_count);
        return vert_count;
#else
        return vert_count;
#endif
    }

    void set_vert_count(const pxr::VtArray<int>& vert_count)
    {
#if USE_USD_SCRATCH_BUFFER
        curves.CreateCurveVertexCountsAttr().Set(vert_count);
#else
        this->vert_count = vert_count;
#endif
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_display_color() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<pxr::GfVec3f> displayColor;
        if (curves.GetDisplayColorAttr())
            curves.GetDisplayColorAttr().Get(&displayColor);
        return displayColor;
#else
        return displayColor;
#endif
    }

    void set_display_color(const pxr::VtArray<pxr::GfVec3f>& display_color)
    {
#if USE_USD_SCRATCH_BUFFER
        curves.CreateDisplayColorAttr().Set(display_color);
#else
        this->displayColor = display_color;
#endif
    }

#if USE_USD_SCRATCH_BUFFER
    pxr::UsdGeomBasisCurves get_usd_curve() const
    {
        return curves;
    }
#endif

    [[nodiscard]] bool get_periodic() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtValue periodic_val;
        if (curves.GetWrapAttr())
            curves.GetWrapAttr().Get(&periodic_val);
        return periodic_val == pxr::UsdGeomTokens->periodic;
#else
        return periodic;
#endif
    }

    void set_periodic(bool is_periodic)
    {
#if USE_USD_SCRATCH_BUFFER
        auto wrap_value = is_periodic ? pxr::UsdGeomTokens->periodic
                                      : pxr::UsdGeomTokens->nonperiodic;
        curves.CreateWrapAttr().Set(wrap_value);
#else
        periodic = is_periodic;
#endif
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_curve_normals() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<pxr::GfVec3f> normals;
        if (curves.GetNormalsAttr())
            curves.GetNormalsAttr().Get(&normals);
        return normals;
#else
        return curve_normals;
#endif
    }

    void set_curve_normals(const pxr::VtArray<pxr::GfVec3f>& normals)
    {
#if USE_USD_SCRATCH_BUFFER
        curves.CreateNormalsAttr().Set(normals);
#else
        curve_normals = normals;
#endif
    }

    GeometryComponentHandle copy(Geometry* operand) const override;

   private:
#if USE_USD_SCRATCH_BUFFER
    pxr::UsdGeomBasisCurves curves;
#else
    pxr::VtArray<pxr::GfVec3f> vertices;
    pxr::VtArray<float> width;
    pxr::VtArray<int> vert_count;
    pxr::VtArray<pxr::GfVec3f> displayColor;

    bool periodic = false;
    pxr::VtArray<pxr::GfVec3f> curve_normals;

#endif
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
