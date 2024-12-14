#pragma once
#include "diff_optics/api.h"
#include "diff_optics/lens_system.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct CompiledDataBlock {
    std::vector<float> parameters;
    std::map<unsigned, unsigned> parameter_offsets;

    unsigned cb_size;

    friend bool operator==(
        const CompiledDataBlock& lhs,
        const CompiledDataBlock& rhs)
    {
        return lhs.parameters == rhs.parameters &&
               lhs.parameter_offsets == rhs.parameter_offsets &&
               lhs.cb_size == rhs.cb_size;
    }

    friend bool operator!=(
        const CompiledDataBlock& lhs,
        const CompiledDataBlock& rhs)
    {
        return !(lhs == rhs);
    }
};

struct LensSystemCompiler {
    LensSystemCompiler()
    {
    }

    static const std::string sphere_intersection;
    static const std::string flat_intersection;
    static const std::string occluder_intersection;
    unsigned indent = 0;
    unsigned cb_offset = 0;
    unsigned cb_size = 0;

    std::string emit_line(
        const std::string& line,
        unsigned cb_size_occupied = 0);

    std::string indent_str(unsigned n)
    {
        return std::string(n, ' ');
    }

    std::tuple<std::string, CompiledDataBlock> compile(
        LensSystem* lens_system,
        bool require_ray_visualization);

    static void fill_block_data(
        LensSystem* lens_system,
        CompiledDataBlock& data_block);
};

class LayerCompiler {
   public:
    virtual ~LayerCompiler() = default;
    explicit LayerCompiler(LensLayer* layer);

    virtual void EmitCbDataLoad(
        int id,
        std::string& constant_buffer,
        std::string& data_load,
        LensSystemCompiler* compiler) = 0;

    virtual void EmitRayTrace(
        int id,
        std::string& execution,
        LensSystemCompiler* compiler) = 0;

    virtual void EmitSampleDirFromSensor(
        int id,
        std::string& sample_from_sensor,
        LensSystemCompiler* compiler) = 0;

    LayerCompiler() = default;

   protected:
    void add_data_load(
        int id,
        std::string& data_load,
        LensSystemCompiler* compiler,
        const std::string& name)
    {
    }

    void add_cb_data_load(
        int id,
        std::string& constant_buffer,
        std::string& data_load,
        LensSystemCompiler* compiler,
        const std::string& name)
    {
        data_load += compiler->emit_line(
            "data." + name + "_" + std::to_string(id) + " = tensor[uint(" +
            std::to_string(compiler->cb_size) + ")]");
        constant_buffer +=
            compiler->emit_line("float " + name + "_" + std::to_string(id), 1);
    }

    LensLayer* layer = nullptr;
};

class NullCompiler : public LayerCompiler {
   public:
    explicit NullCompiler(LensLayer* layer) : LayerCompiler(layer)
    {
    }

    void EmitCbDataLoad(
        int id,
        std::string& constant_buffer,
        std::string& data_load,
        LensSystemCompiler* compiler) override;

    void EmitRayTrace(
        int id,
        std::string& execution,
        LensSystemCompiler* compiler) override;

    void EmitSampleDirFromSensor(
        int id,
        std::string& sample_from_sensor,
        LensSystemCompiler* compiler) override;
};

class OccluderCompiler : public LayerCompiler {
   public:
    explicit OccluderCompiler(LensLayer* layer) : LayerCompiler(layer)
    {
    }

    void EmitCbDataLoad(
        int id,
        std::string& constant_buffer,
        std::string& data_load,
        LensSystemCompiler* compiler) override;

    void EmitRayTrace(
        int id,
        std::string& execution,
        LensSystemCompiler* compiler) override;

    void EmitSampleDirFromSensor(
        int id,
        std::string& sample_from_sensor,
        LensSystemCompiler* compiler) override;
};

class SphericalLensCompiler : public LayerCompiler {
   public:
    explicit SphericalLensCompiler(LensLayer* layer) : LayerCompiler(layer)
    {
    }

    void EmitCbDataLoad(
        int id,
        std::string& constant_buffer,
        std::string& data_load,
        LensSystemCompiler* compiler) override;

    void EmitRayTrace(
        int id,
        std::string& execution,
        LensSystemCompiler* compiler) override;

    void EmitSampleDirFromSensor(
        int id,
        std::string& sample_from_sensor,
        LensSystemCompiler* compiler) override;
};

class FlatLensCompiler : public LayerCompiler {
   public:
    explicit FlatLensCompiler(LensLayer* layer) : LayerCompiler(layer)
    {
    }

    void EmitCbDataLoad(
        int id,
        std::string& constant_buffer,
        std::string& data_load,
        LensSystemCompiler* compiler) override;

    void EmitRayTrace(
        int id,
        std::string& execution,
        LensSystemCompiler* compiler) override;

    void EmitSampleDirFromSensor(
        int id,
        std::string& sample_from_sensor,
        LensSystemCompiler* compiler) override;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE