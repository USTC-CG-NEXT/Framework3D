#pragma once

#include <pxr/base/gf/matrix3f.h>

#include <memory>
#include <vector>

#include "api.h"
#include "imgui.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class LensSystemGUI;
class Pupil;
class LensLayer;
class DiffOpticsGUI;

class OpticalProperty {
   public:
    float refractive_index;
    float abbe_number;
};

struct BBox2D {
    // Default init to an impossible bound
    BBox2D();

    BBox2D(ImVec2 min, ImVec2 max);
    ImVec2 min;
    ImVec2 max;

    ImVec2 center() const;

    BBox2D operator+(const BBox2D& b) const;

    BBox2D& operator+=(const BBox2D& b);
};

class LensGUIPainter {
   public:
    virtual ~LensGUIPainter() = default;
    virtual BBox2D get_bounds(LensLayer*) = 0;
    virtual void
    draw(DiffOpticsGUI* gui, LensLayer*, const pxr::GfMatrix3f& transform) = 0;
};

class LensLayer {
   public:
    LensLayer(float center_x, float center_y);
    virtual ~LensLayer();

    virtual void EmitShader();

    void set_axis(float axis_pos);

    void set_pos(float x);

    ImVec2 center_pos;

   protected:
    std::shared_ptr<LensGUIPainter> painter;
    friend class LensSystemGUI;
};

class PupilPainter;

class Pupil : public LensLayer {
   public:
    explicit Pupil(float radius, float center_x, float center_y);
    float radius;
};

class PupilPainter : public LensGUIPainter {
   public:
    BBox2D get_bounds(LensLayer* layer) override;
    void draw(
        DiffOpticsGUI* gui,
        LensLayer* layer,
        const pxr::GfMatrix3f& transform) override;
};

class LensFilm : public LensLayer {
   public:
    LensFilm(float d, float roc, float center_x, float center_y)
        : LensLayer(center_x, center_y),
          diameter(d),
          radius_of_curvature(roc),
          optical_property()
    {
    }

   private:
    const bool is_spherical = true;

    float diameter;
    float radius_of_curvature;

    std::vector<float> high_order_polynomial_coefficients;

    OpticalProperty optical_property;
};

class LensFilmPainter : public LensGUIPainter {
   public:
    BBox2D get_bounds(LensLayer* layer) override;
    void draw(
        DiffOpticsGUI* gui,
        LensLayer* layer,
        const pxr::GfMatrix3f& transform) override;
};

class LensSystem {
   public:
    LensSystem();

    void add_lens(std::shared_ptr<LensLayer> lens)
    {
        lenses.push_back(lens);
    }

   private:
    std::unique_ptr<LensSystemGUI> gui;
    std::vector<std::shared_ptr<LensLayer>> lenses;

    friend class LensSystemGUI;
};

class LensSystemGUI {
   public:
    explicit LensSystemGUI(LensSystem* lens_system) : lens_system(lens_system)
    {
    }
    virtual ~LensSystemGUI() = default;

    void set_canvas_size(float x, float y);

    virtual void draw(DiffOpticsGUI* gui) const;

   private:
    ImVec2 canvas_size;

    LensSystem* lens_system;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE