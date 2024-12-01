#pragma once

#include <memory>
#include <vector>

#include "../../renderer/nodes/shaders/shaders/utils/ray.slang"
#include "api.h"
#include "io/json.hpp"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct CompiledDataBlock;
class LensSystem;
class LensGUIPainter;
class LayerCompiler;
struct LensSystemCompiler;
class Occluder;
class LensLayer;
class DiffOpticsGUI;

class OpticalProperty {
   public:
    float refractive_index;
    float abbe_number;
};

class LensLayer {
   public:
    LensLayer(float center_x, float center_y);
    virtual ~LensLayer();

    void set_axis(float axis_pos);
    void set_pos(float x);

    virtual void deserialize(const nlohmann::json& j) = 0;

    virtual void fill_block_data(float* ptr) = 0;

    pxr::GfVec2f center_pos;

    OpticalProperty optical_property;

   protected:
    std::unique_ptr<LensGUIPainter> painter;
    std::unique_ptr<LayerCompiler> compiler;
    friend class LensSystemGUI;
    friend class LensSystemCompiler;
};

class NullPainter;
class NullLayer : public LensLayer {
   public:
    NullLayer(float center_x, float center_y);
    void deserialize(const nlohmann::json& j) override;
    void fill_block_data(float* ptr) override;

   private:
    friend class NullPainter;
};

class OccluderPainter;

class Occluder : public LensLayer {
   public:
    explicit Occluder(float radius, float center_x, float center_y);
    void deserialize(const nlohmann::json& j) override;

    void fill_block_data(float* ptr) override;

    float radius;
};

class SphericalLens : public LensLayer {
   public:
    void update_info(float center_x, float center_y);
    SphericalLens(float d, float roc, float center_x, float center_y);
    void deserialize(const nlohmann::json& j) override;

    void fill_block_data(float* ptr) override;

   private:
    float diameter;
    float radius_of_curvature;
    float theta_range;

    pxr::GfVec2f sphere_center;

    friend class SphericalLensPainter;
};

class FlatLens : public LensLayer {
   public:
    FlatLens(float d, float center_x, float center_y);
    void deserialize(const nlohmann::json& j) override;

    void fill_block_data(float* ptr) override;

   private:
    float diameter;
    friend class FlatLensPainter;
};

class LensSystemGUI {
   public:
    explicit LensSystemGUI(LensSystem* lens_system) : lens_system(lens_system)
    {
    }
    virtual ~LensSystemGUI() = default;

    void set_canvas_size(float x, float y);

    void draw_rays(DiffOpticsGUI* gui, const pxr::GfMatrix3f& t) const;
    virtual void draw(DiffOpticsGUI* gui) const;
    void control(DiffOpticsGUI* diff_optics_gui);

   private:
    pxr::GfVec2f canvas_size;

    LensSystem* lens_system;
};

class LensSystem {
   public:
    LensSystem();

    void add_lens(std::shared_ptr<LensLayer> lens);

    size_t lens_count() const
    {
        return lenses.size();
    }

    void deserialize(const std::string& json);
    void deserialize(const std::filesystem::path& path);
    void set_default();

    std::vector<std::vector<RayInfo>> trace_ray(
        const std::vector<RayInfo>& ray_in);

   private:
    std::unique_ptr<LensSystemGUI> gui;
    std::vector<std::shared_ptr<LensLayer>> lenses;

    std::function<std::vector<std::vector<RayInfo>>(
        const std::vector<RayInfo>& ray_in)>
        ray_trace_func;

    std::unique_ptr<CompiledDataBlock> block;
    std::unique_ptr<LensSystemCompiler> compiler;
    void compile_ray_trace_func();

    friend class LensSystemGUI;
    friend class LensSystemCompiler;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE