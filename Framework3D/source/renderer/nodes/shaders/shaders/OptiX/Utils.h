#pragma once


#include <optix.h>


#include <string>
#include <vector>


struct PipelineWithSbt
{
    OptixPipeline pipeline = nullptr;
    OptixShaderBindingTable sbt = {};
};


using OptixShaderFunc = std::tuple<std::string, OptixModule>;

/**
 * \brief IS, CHS, AHS
 */
using HitGroup = std::tuple<OptixShaderFunc,OptixShaderFunc,OptixShaderFunc>;

#define GetEntryName(ShaderFunc) std::get<0>(ShaderFunc)
#define GetModule(ShaderFunc) std::get<1>(ShaderFunc)

#define GetIS(HitGroup) std::get<0>(HitGroup)
#define GetCHS(HitGroup)    std::get<1>(HitGroup)
#define GetAHS(HitGroup)    std::get<2>(HitGroup)

extern char optix_log[2048];

/**
 * \brief
 * \param raygen_name
 * \param hitgroup_names
 * \param miss_names
 * \param optix_modules Make sure the first module contains the raygen shader
 * and all the miss shaders. The rest of the vector should contain one module for each hitgroup.
 * Size should be exactly one more than hitgroup_names.
 * \return
 */
PipelineWithSbt SetupPipelineWithSbt(
    const OptixShaderFunc& raygen_name,
    const std::vector<HitGroup>& hitgroup_names,
    const std::vector<OptixShaderFunc>& miss_names,
    const OptixPipelineCompileOptions& pipeline_compile_options,
    OptixDeviceContext context);

