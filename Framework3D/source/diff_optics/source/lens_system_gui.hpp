#pragma once
#include <pxr/base/gf/matrix3f.h>

#include "diff_optics/api.h"
#include "diff_optics/lens_system.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct BBox2D {
    // Default init to an impossible bound
    BBox2D();

    BBox2D(pxr::GfVec2f min, pxr::GfVec2f max);
    pxr::GfVec2f min;
    pxr::GfVec2f max;

    pxr::GfVec2f center() const;

    BBox2D operator+(const BBox2D& b) const;

    BBox2D& operator+=(const BBox2D& b);
};

class LensGUIPainter {
   public:
    virtual ~LensGUIPainter() = default;
    virtual BBox2D get_bounds(LensLayer*) = 0;
    virtual void
    draw(DiffOpticsGUI* gui, LensLayer*, const pxr::GfMatrix3f& transform) = 0;
    virtual void control(DiffOpticsGUI* diff_optics_gui, LensLayer* get) = 0;

    const char* UniqueUIName(const char* name)
    {
        static char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s##%p", name, this);
        return buffer;
    }
};

class NullPainter : public LensGUIPainter {
   public:
    BBox2D get_bounds(LensLayer* layer) override
    {
        // a box of (1,1) at the center
        return BBox2D{
            pxr::GfVec2f(
                layer->center_pos[0] - 0.5, layer->center_pos[1] - 0.5),
            pxr::GfVec2f(
                layer->center_pos[0] + 0.5, layer->center_pos[1] + 0.5),
        };
    }
    void draw(
        DiffOpticsGUI* gui,
        LensLayer* layer,
        const pxr::GfMatrix3f& transform) override
    {
    }

    void control(DiffOpticsGUI* diff_optics_gui, LensLayer* get) override;
};

class OccluderPainter : public LensGUIPainter {
   public:
    BBox2D get_bounds(LensLayer* layer) override;
    void draw(
        DiffOpticsGUI* gui,
        LensLayer* layer,
        const pxr::GfMatrix3f& transform) override;

    void control(DiffOpticsGUI* diff_optics_gui, LensLayer* get) override;
};

class SphericalLensPainter : public LensGUIPainter {
   public:
    BBox2D get_bounds(LensLayer* layer) override;
    void draw(
        DiffOpticsGUI* gui,
        LensLayer* layer,
        const pxr::GfMatrix3f& transform) override;
    void control(DiffOpticsGUI* diff_optics_gui, LensLayer* get) override;
};

class FlatLensPainter : public LensGUIPainter {
   public:
    BBox2D get_bounds(LensLayer* layer) override;
    void draw(
        DiffOpticsGUI* gui,
        LensLayer* layer,
        const pxr::GfMatrix3f& transform) override;
    void control(DiffOpticsGUI* diff_optics_gui, LensLayer* get) override;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE