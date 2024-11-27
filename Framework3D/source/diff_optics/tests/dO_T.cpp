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

class CPURWTexture : public CPPPrelude::IRWTexture {
   public:
    CPURWTexture(unsigned width, unsigned height, unsigned format_size)
        : data(new char8_t[width * height * format_size]),
          width(width),
          height(height),
          format_size(format_size)
    {
    }

    virtual ~CPURWTexture()
    {
        delete[] data;
    }

    CPPPrelude::TextureDimensions GetDimensions(int mipLevel) override
    {
        return { width, height, 1 };
    }

    void Load(const int32_t* v, void* outData, size_t dataSize) override
    {
        memcpy(outData, data + v[0] * format_size, dataSize);
    }

    void Sample(
        CPPPrelude::SamplerState samplerState,
        const float* loc,
        void* outData,
        size_t dataSize) override
    {
        memcpy(outData, data, dataSize);
    }

    void SampleLevel(
        CPPPrelude::SamplerState samplerState,
        const float* loc,
        float level,
        void* outData,
        size_t dataSize) override
    {
        memcpy(outData, data, dataSize);
    }

    void* refAt(const uint32_t* loc) override
    {
        return data + loc[0] * format_size;
    }

   private:
    char8_t* data;
    unsigned width;
    unsigned height;
    unsigned format_size;
};

TEST(dO_T, gen_shader)
{
    LensSystem lens_system;

    // load from file lens.json
    std::ifstream json_file("lens.json");
    std::string json(
        (std::istreambuf_iterator<char>(json_file)),
        std::istreambuf_iterator<char>());

    lens_system.deserialize(json);
    LensSystemCompiler compiler;
    auto [shader_str, compiled_block] = compiler.compile(&lens_system, true);

    // Save file
    std::ofstream file("lens_shader.slang");
    file << shader_str;
    file.close();

    ShaderFactory shader_factory;
    shader_factory.set_search_path("../../source/renderer/nodes/shaders");

    ShaderReflectionInfo reflection;
    std::string error_string;
    auto program_handle = shader_factory.compile_cpu_executable(
        "computeMain",
        nvrhi::ShaderType::Compute,
        "shaders/physical_lens_raygen.slang",
        reflection,
        error_string);

    unsigned width = 360;
    unsigned height = 240;
    CPPPrelude::ComputeVaryingInput input;
    input.startGroupID = { 0, 0, 0 };
    input.endGroupID = { width, height, 1 };

    struct UniformState {
        CPPPrelude::RWStructuredBuffer<RayInfo> rays;
        CPPPrelude::RWTexture2D<CPPPrelude::uint> random_seeds;

        CPPPrelude::RWStructuredBuffer<CPPPrelude::uint2> pixel_targets;
        CPPPrelude::uint2* size;

        void* data;
        CPPPrelude::RWStructuredBuffer<RayInfo> ray_visualizations[20];

    } state;

    std::vector<RayInfo> rays(width * height, RayInfo());
    state.rays.data = rays.data();
    state.rays.count = rays.size();
    state.random_seeds.texture =
        new CPURWTexture(width, height, sizeof(CPPPrelude::uint));

    std::vector<std::vector<RayInfo>> ray_visualizations;
    for (size_t i = 0; i < lens_system.lens_count(); i++) {
        ray_visualizations.push_back(std::vector<RayInfo>(width * height));
        state.ray_visualizations[i].data = ray_visualizations[i].data();
        state.ray_visualizations[i].count = ray_visualizations[i].size();
    }

    std::vector<CPPPrelude::uint2> pixel_targets(width * height, 1);
    state.pixel_targets.data = pixel_targets.data();
    state.pixel_targets.count = pixel_targets.size();

    CPPPrelude::uint2 size = { width, height };
    state.size = &size;
    state.data = compiled_block.parameters.data();

    compiled_block.parameters[0] = 36;
    compiled_block.parameters[1] = 24;

    compiled_block.parameters[2] = *reinterpret_cast<float*>(&width);
    compiled_block.parameters[3] = *reinterpret_cast<float*>(&height);
    compiled_block.parameters[4] = 50.0f;

    std::cout << error_string << std::endl;
    std::cout << reflection << std::endl;

    program_handle->host_call(input, state);

    std::vector<pxr::GfVec3f> rays_orgs;
    for (int i = 0; i < rays.size(); ++i) {
        rays_orgs.push_back(rays[i].Origin);
    }

    std::vector<pxr::GfVec3f> ray_dirs;
    for (int i = 0; i < rays.size(); ++i) {
        ray_dirs.push_back(rays[i].Direction);
    }

    // paint to a ppm image

    for (size_t i = 0; i < lens_system.lens_count(); i++) {
        std::ofstream file(
            "ray_visualization_dirs_" + std::to_string(i) + ".ppm");

        file << "P3\n" << width << " " << height << "\n255\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                RayInfo ray = ray_visualizations[i][y * width + x];

                if (!isnan(ray.Direction[0])) {
                    int r = static_cast<int>(
                        (ray.Direction[0] + 1.0f) * 0.5f * 255.0f);
                    int g = static_cast<int>(
                        (ray.Direction[1] + 1.0f) * 0.5f * 255.0f);
                    int b = static_cast<int>(
                        (ray.Direction[2] + 1.0f) * 0.5f * 255.0f);
                    file << std::clamp(r, 0, 255) << " "
                         << std::clamp(g, 0, 255) << " "
                         << std::clamp(b, 0, 255) << "\n";
                }
                else {
                    file << "0 0 0"
                         << "\n";
                }
            }
        }
        file.close();
    }

    for (size_t i = 0; i < lens_system.lens_count(); i++) {
        std::ofstream file(
            "ray_visualization_orgs_" + std::to_string(i) + ".ppm");

        file << "P3\n" << width << " " << height << "\n255\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                RayInfo ray = ray_visualizations[i][y * width + x];

                if (!isnan(ray.Origin[0])) {
                    int r = static_cast<int>(
                        (ray.Origin[0] + 1.0f) * 0.5f * 255.0f);
                    int g = static_cast<int>(
                        (ray.Origin[1] + 1.0f) * 0.5f * 255.0f);
                    int b = static_cast<int>((ray.Origin[2] * 0));
                    file << std::clamp(r, 0, 255) << " "
                         << std::clamp(g, 0, 255) << " "
                         << std::clamp(b, 0, 255) << "\n";
                }
                else {
                    file << "0 0 0"
                         << "\n";
                }
            }
        }
        file.close();
    }

    std::ofstream file2("ray_dirs.ppm");

    file2 << "P3\n" << width << " " << height << "\n255\n";
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            pxr::GfVec3f dir = ray_dirs[y * width + x];

            if (!isnan(dir[0])) {
                int r = static_cast<int>((dir[0] + 1.0f) * 0.5f * 255.0f);
                int g = static_cast<int>((dir[1] + 1.0f) * 0.5f * 255.0f);
                int b = static_cast<int>((dir[2] + 1.0f) * 0.5f * 255.0f);
                file2 << r << " " << g << " " << b << "\n";
            }
            else {
                file2 << "0 0 0"
                      << "\n";
            }
        }
    }
    file2.close();

    Window window;
}