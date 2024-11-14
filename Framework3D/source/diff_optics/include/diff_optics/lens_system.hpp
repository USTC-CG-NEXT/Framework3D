#pragma once

#include <pxr/base/gf/matrix3f.h>

#include <memory>
#include <vector>

#include "api.h"
#include "imgui.h"
#include "io/json.hpp"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class LensSystemGUI;
class Occluder;
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

class LensLayer {
   public:
    LensLayer(float center_x, float center_y);
    virtual ~LensLayer();
    virtual void
    EmitShader(int id, std::string& constant_buffer, std::string& execution)
    {
    }
    void set_axis(float axis_pos);
    void set_pos(float x);

    virtual void deserialize(const nlohmann::json& j) = 0;

    virtual unsigned get_cb_size()
    {
        return cb_size;
    }

    pxr::GfVec2f center_pos;

    OpticalProperty optical_property;

   protected:
    std::string emit_line(const std::string& line, unsigned cb_size_occupied);

    static unsigned cb_offset;
    unsigned cb_size;

    std::shared_ptr<LensGUIPainter> painter;
    friend class LensSystemGUI;
};

class NullPainter;
class NullLayer : public LensLayer {
   public:
    NullLayer(float center_x, float center_y);
    void deserialize(const nlohmann::json& j) override;
    unsigned get_cb_size() override;

   private:
    friend class NullPainter;
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

class OccluderPainter;

class Occluder : public LensLayer {
   public:
    explicit Occluder(float radius, float center_x, float center_y);
    void deserialize(const nlohmann::json& j) override;

    void EmitShader(
        int id,
        std::string& constant_buffer,
        std::string& execution) override;

    unsigned get_cb_size() override;

    float radius;
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

class SphericalLens : public LensLayer {
   public:
    void update_info(float center_x, float center_y);
    SphericalLens(float d, float roc, float center_x, float center_y);
    void deserialize(const nlohmann::json& j) override;
    void EmitShader(
        int id,
        std::string& constant_buffer,
        std::string& execution) override;
    unsigned get_cb_size() override;

   private:
    float diameter;
    float radius_of_curvature;
    float theta_range;

    pxr::GfVec2f sphere_center;

    friend class SphericalLensPainter;
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

class FlatLens : public LensLayer {
   public:
    FlatLens(float d, float center_x, float center_y);
    void deserialize(const nlohmann::json& j) override;
    unsigned get_cb_size() override;

    void
    EmitShader(int id, std::string& constant_buffer, std::string& execution);

   private:
    float diameter;
    friend class FlatLensPainter;
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

class LensSystem {
   public:
    LensSystem();

    void add_lens(std::shared_ptr<LensLayer> lens);

    unsigned get_cb_size() const
    {
        return cb_size;
    }

    void deserialize(const std::string& json);

   private:
    unsigned cb_size = 0;
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
    void control(DiffOpticsGUI* diff_optics_gui);

   private:
    pxr::GfVec2f canvas_size;

    LensSystem* lens_system;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE