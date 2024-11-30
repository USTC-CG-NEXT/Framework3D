#include "compute_context.hpp"

#include "nvrhi/utils.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
ComputeContext::ComputeContext(ResourceAllocator& r, ProgramVars& vars)
    : GPUContext(r, vars)

{
    auto programs = vars.get_programs();

    assert(programs.size() == 1);
    auto program = programs[0];
    cs_shader = resource_allocator_.create(
        program->get_shader_desc(),
        program->getBufferPointer(),
        program->getBufferSize());

    compute_pipeline_desc.CS = cs_shader;
}

ComputeContext::~ComputeContext()
{
    resource_allocator_.destroy(compute_pipeline);
    resource_allocator_.destroy(cs_shader);
}

inline int div_ceil(int dividend, int divisor)
{
    return (dividend + (divisor - 1)) / divisor;
}

void ComputeContext::dispatch(
    const GraphicsComputeState& s,
    const ProgramVars& program_vars,
    uint32_t width,
    uint32_t groupSizeX,
    uint32_t height,
    uint32_t groupSizeY,
    uint32_t depth,
    uint32_t groupSizeZ) const
{
    nvrhi::ComputeState state;
    state.indirectParams = s.indirectParams;
    state.pipeline = compute_pipeline;
    state.bindings = program_vars.get_binding_sets();
    commandList_->setComputeState(state);
    commandList_->dispatch(
        div_ceil(width, groupSizeX),
        div_ceil(height, groupSizeY),
        div_ceil(depth, groupSizeZ));
}

ComputeContext& ComputeContext::finish_setting_pso()
{
    auto bindingLayouts = vars_.get_binding_layout();

    compute_pipeline_desc.bindingLayouts = bindingLayouts;
    compute_pipeline = resource_allocator_.create(compute_pipeline_desc);

    return *this;
}

void ComputeContext::uav_barrier(nvrhi::ITexture* texture) const
{
    nvrhi::utils::TextureUavBarrier(commandList_, texture);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE