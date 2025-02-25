#pragma once
#include <string>

#include "GCore/Components.h"
#include "GCore/GOP.h"
#include "pxr/usd/usdGeom/points.h"
#include "pxr/usd/usdGeom/xform.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct GEOMETRY_API PointsComponent : public GeometryComponent {
    explicit PointsComponent(Geometry* attached_operand);

    std::string to_string() const override;

    void apply_transform(const pxr::GfMatrix4d& transform) override
    {
        auto vertices = get_vertices();
        for (auto& vertex : vertices) {
            vertex = pxr::GfVec3f(transform.Transform(vertex));
        }
        set_vertices(vertices);
    }

    GeometryComponentHandle copy(Geometry* operand) const override;

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_vertices() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<pxr::GfVec3f> vertices;
        if (points.GetPointsAttr())
            points.GetPointsAttr().Get(&vertices);
#endif
        return vertices;
    }

    [[nodiscard]] pxr::VtArray<pxr::GfVec3f> get_display_color() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<pxr::GfVec3f> displayColor;
        if (points.GetDisplayColorAttr())
            points.GetDisplayColorAttr().Get(&displayColor);
#endif
        return displayColor;
    }

    [[nodiscard]] pxr::VtArray<float> get_width() const
    {
#if USE_USD_SCRATCH_BUFFER
        pxr::VtArray<float> width;
        if (points.GetWidthsAttr())
            points.GetWidthsAttr().Get(&width);
#endif
        return width;
    }

    void set_vertices(const pxr::VtArray<pxr::GfVec3f>& vertices)
    {
#if USE_USD_SCRATCH_BUFFER
        points.CreatePointsAttr().Set(vertices);
#else
        this->vertices = vertices;
#endif
    }

    void set_display_color(const pxr::VtArray<pxr::GfVec3f>& display_color)
    {
#if USE_USD_SCRATCH_BUFFER
        points.CreateDisplayColorAttr().Set(display_color);
#else
        this->displayColor = display_color;
#endif
    }

    void set_width(const pxr::VtArray<float>& width)
    {
#if USE_USD_SCRATCH_BUFFER
        points.CreateWidthsAttr().Set(width);
#else
        this->width = width;
#endif
    }

#if USE_USD_SCRATCH_BUFFER
    pxr::UsdGeomPoints get_usd_points() const
    {
        return points;
    }
#endif

   private:
#if USE_USD_SCRATCH_BUFFER
    pxr::UsdGeomPoints points;
#else
    pxr::VtArray<pxr::GfVec3f> vertices;
    pxr::VtArray<pxr::GfVec3f> displayColor;
    pxr::VtArray<float> width;
#endif
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
