#include <GUI/window.h>
#include <gtest/gtest.h>

#include <diff_optics/diff_optics.hpp>
#include <fstream>

#include "../../renderer/nodes/shaders/shaders/utils/PhysicalCamInfo.h"
#include "../../renderer/nodes/shaders/shaders/utils/ray.slang"
#include "RHI/ShaderFactory/shader.hpp"
#include "diff_optics/lens_system.hpp"
#include "diff_optics/lens_system_compiler.hpp"

using namespace USTC_CG;

#include "slang-cpp-types.h"

TEST(dO_T, gen_shader)
{
    LensSystem lens_system;
    lens_system.set_default();

    LensSystemCompiler compiler;
    auto [shader_str, compiled_block] = compiler.compile(&lens_system, true);

    // Save file
    std::ofstream file("lens_shader.slang");
    file << shader_str;
    file.close();
}

std::ostream& operator<<(std::ostream& os, const RayInfo& ray)
{
    os << "RayInfo{";
    os << "Origin: " << ray.Origin << ", ";
    os << "Direction: " << ray.Direction << ", ";
    os << "TMin: " << ray.TMin << ", ";
    os << "TMax: " << ray.TMax << ", ";
    os << "throughput: " << ray.throughput.data;
    os << "}";
    return os;
}

TEST(dO_T, gen_shader_run)
{
    LensSystem lens_system;
    lens_system.set_default();

    RayInfo begin;
    begin.Origin = { 0, 0, -2 };
    begin.Direction = { 0, 0, 1 };
    begin.TMin = 0;
    begin.TMax = 1000;
    begin.throughput.data = pxr::GfVec3f{ 0.8, 0.7, 0.8 };

    auto result_rays = lens_system.trace_ray({ begin });
    for (auto& ray_step : result_rays) {
        for (auto& ray : ray_step) {
            std::cout << ray << std::endl;
        }
    }
}
