#pragma once

#include <memory>
#include <vector>

#include "api.h"
#include "imgui.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class Pupil;
class LensLayer;
class DiffOpticsGUI;

class OpticalProperty {
   public:
    float refractive_index;
    float abbe_number;
};

class LensGUIPainter {
   public:
    virtual ~LensGUIPainter() = default;

    virtual void draw(DiffOpticsGUI* gui, LensLayer*, ImVec2 overall_bias) = 0;
};

class LensLayer {
   public:
    LensLayer(float center_x, float center_y) : center_pos(center_x, center_y)
    {
    }
    virtual ~LensLayer() = default;

    virtual void EmitShader()
    {
    }

    void set_axis(float axis_pos)
    {
        center_pos.y = axis_pos;
    }

    void set_pos(float x)
    {
        center_pos.x = x;
    }

    ImVec2 center_pos;

   protected:
    std::shared_ptr<LensGUIPainter> painter;
    friend class LensSystem;
};

class PupilPainter;

class Pupil : public LensLayer {
   public:
    explicit Pupil(float radius, float center_x, float center_y);
    float radius;
};

class PupilPainter : public LensGUIPainter {
   public:
    void draw(DiffOpticsGUI* gui, LensLayer* layer, ImVec2 overall_bias)
        override;
};

class LensFilm : public LensLayer {
   public:
    LensFilm(float center_x, float center_y) : LensLayer(center_x, center_y)
    {
    }

   private:
    const bool is_spherical = true;

    float radii;
    std::vector<float> high_order_polynomial_coefficients;

    OpticalProperty optical_property;
};

class LensSystem {
   public:
    void add_lens(std::shared_ptr<LensLayer> lens)
    {
        lenses.push_back(lens);
    }

    void draw(DiffOpticsGUI* gui) const;

   private:
    std::vector<std::shared_ptr<LensLayer>> lenses;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE