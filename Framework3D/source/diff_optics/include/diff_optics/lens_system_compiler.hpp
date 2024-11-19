#pragma once
#include "diff_optics/api.h"
#include "diff_optics/lens_system.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct CompiledDataBlock {
    std::vector<float> parameters;
    std::map<unsigned, unsigned> parameter_offsets;

    unsigned cb_size;
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

USTC_CG_NAMESPACE_CLOSE_SCOPE