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
    static unsigned indent;
    static unsigned cb_offset;
    static unsigned cb_size;

    static std::string emit_line(
        const std::string& line,
        unsigned cb_size_occupied = 0);

    static std::string indent_str(unsigned n)
    {
        return std::string(n, ' ');
    }

    static std::tuple<std::string, CompiledDataBlock> compile(
        LensSystem* lens_system);
};

USTC_CG_NAMESPACE_CLOSE_SCOPE