#pragma once
#include "diff_optics/api.h"
#include "diff_optics/lens_system.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

struct LensSystemCompiler {
    LensSystemCompiler()
    {
    }

    static const std::string sphere_intersection;
    static const std::string flat_intersection;
    static unsigned indent;

    static std::string indent_str(unsigned n)
    {
        return std::string(n, ' ');
    }

    static std::string compile(LensSystem* lens_system);
};

USTC_CG_NAMESPACE_CLOSE_SCOPE