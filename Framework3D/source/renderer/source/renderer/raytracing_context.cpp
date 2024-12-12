#include "raytracing_context.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
RaytracingContext::RaytracingContext(ResourceAllocator& r, ProgramVars& vars)
    : GPUContext(r, vars)
{
}

RaytracingContext::~RaytracingContext()
{
}

void RaytracingContext::begin()
{
    GPUContext::begin();
}

void RaytracingContext::finish()
{
    GPUContext::finish();
}

void RaytracingContext::trace_rays(
    const nvrhi::rt::State& state,
    const ProgramVars& program_vars,
    uint32_t width,
    uint32_t height,
    uint32_t depth) const
{
}

void RaytracingContext::set_ray_generation_shader(
    const nvrhi::ShaderHandle& shader)
{
}

void RaytracingContext::set_miss_shader(const nvrhi::ShaderHandle& shader)
{
}

void RaytracingContext::set_hit_group(
    const std::string& name,
    const nvrhi::ShaderHandle& closestHit,
    const nvrhi::ShaderHandle& anyHit,
    const nvrhi::ShaderHandle& intersection)
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE