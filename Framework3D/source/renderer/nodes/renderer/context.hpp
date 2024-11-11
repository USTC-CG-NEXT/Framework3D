#pragma once

#include "RHI/ResourceManager/resource_allocator.hpp"
#include "nvrhi/nvrhi.h"
#include "program_vars.hpp"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class GPUContext {
   public:
    virtual ~GPUContext() = default;

    virtual void finish();

   protected:
    ResourceAllocator& resource_allocator_;
    ProgramVars& vars_;
    nvrhi::CommandListHandle commandList_;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE