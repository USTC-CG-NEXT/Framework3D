#include <gtest/gtest.h>

#include <fstream>

#include "RHI/ShaderFactory/shader.hpp"

using namespace USTC_CG;

const char* str = R"(

RWStructuredBuffer<float> ioBuffer;
RWStructuredBuffer<float> t_BindlessBuffers[] : register(space1);

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    float i = ioBuffer[tid];
    float o = i < 0.5 ? (i + i) : sqrt(i);

    ioBuffer[tid] = o;
}

)";

TEST(cpu_call, gen_shader)
{
    std::string shader_str = str;

    ShaderFactory shader_factory;

    ShaderReflectionInfo reflection;
    std::string error_string;
    auto program_handle = shader_factory.compile_shader(
        "computeMain",
        nvrhi::ShaderType::Compute,
        "",
        reflection,
        error_string,
        {},
        shader_str);


    std::cout << error_string << std::endl;
    std::cout << reflection << std::endl;
}