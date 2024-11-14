#include <GUI/window.h>
#include <gtest/gtest.h>

#include <diff_optics/diff_optics.hpp>
#include <fstream>

#include "../../renderer/nodes/shaders/shaders/utils/ray.h"
#include "RHI/ShaderFactory/shader.hpp"
#include "diff_optics/lens_system.hpp"

using namespace USTC_CG;

const char* str = R"(
{
    "Originate": "US02532751-1",
    "data": [
        {
            "type": "O",
            "distance": 0.0,
            "roc": 0.0,
            "diameter": 0.0,
            "material": "VACUUM"
        },
        {
            "type": "S",
            "distance": 0.0,
            "roc": 13.354729316461547,
            "diameter": 17.4,
            "material": "SSK4",
            "additional_params": [0.005, 1e-6, 1e-8, -3e-10]
        },
        {
            "type": "S",
            "distance": 2.2352,
            "roc": 35.64148197667863,
            "diameter": 17.4,
            "material": "VACUUM"
        },
        {
            "type": "S",
            "distance": 0.0762,
            "roc": 10.330017837998932,
            "diameter": 14.0,
            "material": "SK1"
        },
        {
            "type": "S",
            "distance": 3.1750,
            "roc": 0.0,
            "diameter": 14.0,
            "material": "F15"
        },
        {
            "type": "S",
            "distance": 0.9652,
            "roc": 6.494496063151893,
            "diameter": 9.0,
            "material": "VACUUM"
        },
        {
            "type": "A",
            "distance": 3.8608,
            "roc": 0.0,
            "diameter": 4.886,
            "material": "OCCLUDER"
        },
        {
            "type": "S",
            "distance": 3.302,
            "roc": -7.026950339915501,
            "diameter": 9.0,
            "material": "F15"
        },
        {
            "type": "S",
            "distance": 0.9652,
            "roc": 0.0,
            "diameter": 12.0,
            "material": "SK16"
        },
        {
            "type": "S",
            "distance": 2.7686,
            "roc": -9.746574604143909,
            "diameter": 12.0,
            "material": "VACUUM"
        },
        {
            "type": "S",
            "distance": 0.0762,
            "roc": 69.81692521236866,
            "diameter": 14.0,
            "material": "SK16"
        },
        {
            "type": "S",
            "distance": 1.7526,
            "roc": -19.226275376106166,
            "diameter": 14.0,
            "material": "VACUUM"
        }
    ]
}

)";

#include "slang-cpp-types.h"


class CPURWTexture : public IRWTexture {
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

    TextureDimensions GetDimensions(int mipLevel) override
    {
        return { width, height, 1 };
    }

    void Load(const int32_t* v, void* outData, size_t dataSize) override
    {
        memcpy(outData, data + v[0] * format_size, dataSize);
    }

    void Sample(
        SamplerState samplerState,
        const float* loc,
        void* outData,
        size_t dataSize) override
    {
        memcpy(outData, data, dataSize);
    }

    void SampleLevel(
        SamplerState samplerState,
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

    std::string json = std::string(str);
    lens_system.deserialize(json);
    std::string shader_str = lens_system.gen_slang_shader();

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

    ComputeVaryingInput input;
    input.startGroupID = { 0, 0, 0 };
    input.endGroupID = { 1, 1, 1 };

    struct UniformState {
        RWStructuredBuffer<RayInfo> rays;
        RWTexture2D<uint> random_seeds;

        RWStructuredBuffer<uint2> pixel_targets;
        uint2* size;

        void* data;
    } state;

    std::vector<RayInfo> rays(1024, RayInfo());
    state.rays.data = rays.data();
    state.rays.count = rays.size();
    state.random_seeds.texture = new CPURWTexture(1024, 1024, sizeof(uint));

    std::vector<uint2> pixel_targets(1024, 1);
    state.pixel_targets.data = pixel_targets.data();
    state.pixel_targets.count = pixel_targets.size();

    uint2 size = { 1024, 1 };
    state.size = &size;

    std::vector<uint8_t> lens_data(lens_system.get_cb_size());
    state.data = lens_data.data();

    std::cout << error_string << std::endl;
    std::cout << reflection << std::endl;

    program_handle->host_call(input, state);

    Window window;
}