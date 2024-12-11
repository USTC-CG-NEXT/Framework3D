#include <gtest/gtest.h>

#include <fstream>

#include "RHI/ShaderFactory/shader.hpp"

using namespace USTC_CG;

const char* str = R"(

RWStructuredBuffer<float> ioBuffer;
RWStructuredBuffer<float> t_BindlessBuffers[] : register(t0, space1);

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

const char* str2 = R"(

RWStructuredBuffer<float> ioBuffer;

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

TEST(cpu_call, gen_shader2)
{
    std::string shader_str = str2;
    ShaderFactory shader_factory;
    ShaderReflectionInfo reflection;
    std::string error_string;
    auto program_handle = shader_factory.compile_cpu_executable(
        "computeMain",
        nvrhi::ShaderType::Compute,
        "",
        reflection,
        error_string,
        {},
        shader_str);

    const CPPPrelude::uint3 startGroupID = { 0, 0, 0 };
    const CPPPrelude::uint3 endGroupID = { 1, 1, 1 };

    CPPPrelude::ComputeVaryingInput varyingInput;
    varyingInput.startGroupID = startGroupID;
    varyingInput.endGroupID = endGroupID;

    // We don't have any entry point parameters so that's passed as NULL
    // We need to cast our definition of the uniform state to the undefined
    // CPPPrelude::UniformState as that type is just a name to indicate what
    // kind of thing needs to be passed in.
    // the uniformState will be passed as a pointer to the CPU code

    struct UniformState {
        CPPPrelude::RWStructuredBuffer<float> ioBuffer;
    };
    UniformState uniformState;

    // The contents of the buffer are modified, so we'll copy it
    const float startBufferContents[] = { 2.0f, -10.0f, -3.0f, 5.0f };
    float bufferContents[SLANG_COUNT_OF(startBufferContents)];
    memcpy(bufferContents, startBufferContents, sizeof(startBufferContents));

    // Set up the ioBuffer such that it uses bufferContents. It is important to
    // set the .count such that bounds checking can be performed in the kernel.
    uniformState.ioBuffer.data = bufferContents;
    uniformState.ioBuffer.count = SLANG_COUNT_OF(bufferContents);

    program_handle->host_call(varyingInput, uniformState);

    // Print out the values before the computation
    printf("Before:\n");
    for (float v : startBufferContents) {
        printf("%f, ", v);
    }
    printf("\n");

    // Print out the values the the kernel produced
    printf("After: \n");
    for (float v : bufferContents) {
        printf("%f, ", v);
    }
    printf("\n");
}
