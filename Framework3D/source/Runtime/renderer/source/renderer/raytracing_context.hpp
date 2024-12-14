#pragma once

#include "context.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

struct HD_USTC_CG_API RaytracingState { };

class HD_USTC_CG_API RaytracingContext : public GPUContext {
   public:
    explicit RaytracingContext(ResourceAllocator& r, ProgramVars& vars);
    ~RaytracingContext() override;

    void begin() override;
    void finish() override;

    void trace_rays(
        const RaytracingState& state,
        const ProgramVars& program_vars,
        uint32_t width,
        uint32_t height = 1,
        uint32_t depth = 1) const;

    void announce_raygeneration(const std::string& name);
    void announce_hitgroup(
        const std::string& closesthit,
        const std::string& anyhit = "",
        const std::string& intercestion = "",

        unsigned position = 0);

    void announce_miss(const std::string& name, unsigned position = 0);

    void finish_announcing_shader_names();

   private:
    // Shader names
    std::string raygeneration_name;
    std::vector<std::tuple<std::string, std::string, std::string>>
        hitgroup_names;
    std::vector<std::string> miss_names;

    // Solid shaders
    nvrhi::ShaderHandle ray_generation_shader;
    std::vector<std::tuple<ShaderHandle, ShaderHandle, ShaderHandle>>
        hit_groups;
    std::vector<nvrhi::ShaderHandle> miss_shaders;

    // Pipeline
    IProgram* program;
    nvrhi::rt::ShaderTableHandle sbt;
    nvrhi::rt::PipelineHandle raytracing_pipeline;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
