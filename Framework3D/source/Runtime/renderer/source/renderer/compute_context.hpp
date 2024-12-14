#pragma once

#include "RHI/ResourceManager/resource_allocator.hpp"
#include "context.hpp"
#include "nvrhi/nvrhi.h"
#include "program_vars.hpp"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct GraphicsComputeState {
    nvrhi::IBuffer* indirectParams;
};

class HD_USTC_CG_API ComputeContext : public GPUContext {
   public:
    explicit ComputeContext(ResourceAllocator& r, ProgramVars& vars);
    ~ComputeContext() override;

    void dispatch(
        const GraphicsComputeState& state,
        const ProgramVars& program_vars,
        uint32_t width = 1,
        uint32_t groupSizeX = 1,
        uint32_t height = 1,
        uint32_t groupSizeY = 1,
        uint32_t depth = 1,
        uint32_t groupSizeZ = 1) const;

    ComputeContext& finish_setting_pso();
    void uav_barrier(nvrhi::ITexture* texture) const;

   protected:
    nvrhi::ComputePipelineDesc compute_pipeline_desc;
    nvrhi::ComputePipelineHandle compute_pipeline;
    nvrhi::ShaderHandle cs_shader;
    nvrhi::ComputeState compute_state;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
