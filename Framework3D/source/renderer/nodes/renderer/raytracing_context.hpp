//#pragma once
//
//#include "context.hpp"
//
//USTC_CG_NAMESPACE_OPEN_SCOPE
//
//class RaytracingContext : public GPUContext {
//   public:
//    explicit RaytracingContext(ResourceAllocator& r, ProgramVars& vars);
//    ~RaytracingContext() override;
//
//    void begin() override;
//    void finish() override;
//
//    void trace_rays(
//        const nvrhi::rt::State& state,
//        const ProgramVars& program_vars,
//        uint32_t width,
//        uint32_t height,
//        uint32_t depth) const;
//
//    void set_ray_generation_shader(const nvrhi::ShaderHandle& shader);
//    void set_miss_shader(const nvrhi::ShaderHandle& shader);
//    void set_hit_group(
//        const std::string& name,
//        const nvrhi::ShaderHandle& closestHit,
//        const nvrhi::ShaderHandle& anyHit,
//        const nvrhi::ShaderHandle& intersection);
//
//   private:
//    nvrhi::rt::PipelineHandle raytracing_pipeline;
//    nvrhi::ShaderHandle ray_generation_shader;
//    nvrhi::ShaderHandle miss_shader;
//};
//
//USTC_CG_NAMESPACE_CLOSE_SCOPE
