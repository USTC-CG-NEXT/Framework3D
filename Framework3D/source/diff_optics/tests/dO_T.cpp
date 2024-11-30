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
//
//TEST(dO_T, gen_shader_run)
//{
//    LensSystem lens_system;
//    lens_system.set_default();
//
//    LensSystemCompiler compiler;
//    auto [shader_str, compiled_block] = compiler.compile(&lens_system, true);
//
//    // Save file
//    std::ofstream file("lens_shader.slang");
//    file << shader_str;
//    file.close();
//    ShaderFactory shader_factory;
//    shader_factory.set_search_path("../../source/renderer/nodes/shaders");
//
//    ShaderReflectionInfo reflection;
//    std::string error_string;
//    auto program_handle = shader_factory.compile_cpu_executable(
//        "computeMain",
//        nvrhi::ShaderType::Compute,
//        "shaders/physical_lens_raygen_cpu.slang",
//        reflection,
//        error_string);
//
//    unsigned width = 360;
//    unsigned height = 240;
//    CPPPrelude::ComputeVaryingInput input;
//    input.startGroupID = { 0, 0, 0 };
//    input.endGroupID = { width, height, 1 };
//
//    struct UniformState {
//        CPPPrelude::RWStructuredBuffer<RayInfo> rays;
//        CPPPrelude::RWTexture2D<CPPPrelude::uint> random_seeds;
//
//        CPPPrelude::RWStructuredBuffer<CPPPrelude::uint2> pixel_targets;
//        CPPPrelude::uint2* size;
//
//        void* data;
//        CPPPrelude::RWStructuredBuffer<RayInfo> ray_visualizations[20];
//
//    } state;
//
//    std::vector<RayInfo> rays(width * height, RayInfo());
//    state.rays.data = rays.data();
//    state.rays.count = rays.size();
//    state.random_seeds.texture =
//        new CPURWTexture(width, height, sizeof(CPPPrelude::uint));
//
//    std::vector<std::vector<RayInfo>> ray_visualizations;
//    for (size_t i = 0; i < lens_system.lens_count(); i++) {
//        ray_visualizations.push_back(std::vector<RayInfo>(width * height));
//        state.ray_visualizations[i].data = ray_visualizations[i].data();
//        state.ray_visualizations[i].count = ray_visualizations[i].size();
//    }
//
//    std::vector<CPPPrelude::uint2> pixel_targets(width * height, 1);
//    state.pixel_targets.data = pixel_targets.data();
//    state.pixel_targets.count = pixel_targets.size();
//
//    CPPPrelude::uint2 size = { width, height };
//    state.size = &size;
//    state.data = compiled_block.parameters.data();
//
//    compiled_block.parameters[0] = 36;
//    compiled_block.parameters[1] = 24;
//
//    compiled_block.parameters[2] = *reinterpret_cast<float*>(&width);
//    compiled_block.parameters[3] = *reinterpret_cast<float*>(&height);
//    compiled_block.parameters[4] = 50.0f;
//
//    std::cout << error_string << std::endl;
//    std::cout << reflection << std::endl;
//
//    program_handle->host_call(input, state);
//
//    std::vector<pxr::GfVec3f> rays_orgs;
//    for (int i = 0; i < rays.size(); ++i) {
//        rays_orgs.push_back(rays[i].Origin);
//    }
//
//    std::vector<pxr::GfVec3f> ray_dirs;
//    for (int i = 0; i < rays.size(); ++i) {
//        ray_dirs.push_back(rays[i].Direction);
//    }
//
//    Window window;
//}