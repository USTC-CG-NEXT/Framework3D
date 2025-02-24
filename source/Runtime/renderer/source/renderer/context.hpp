#pragma once
#include "api.h"
#include "RHI/ResourceManager/resource_allocator.hpp"
#include "nvrhi/nvrhi.h"
#include "program_vars.hpp"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class HD_USTC_CG_API GPUContext {
   public:
    virtual ~GPUContext();

    GPUContext(ResourceAllocator& r, ProgramVars& vars);

    virtual void begin();
    virtual void finish();

    void clear_buffer(
        nvrhi::IBuffer* buffer,
        uint32_t clear_value = 0,
        const nvrhi::BufferRange& range = nvrhi::EntireBuffer);

    void clear_texture(
        nvrhi::ITexture* texture,
        nvrhi::Color color = { 0 },
        const nvrhi::TextureSubresourceSet& subresources =
            nvrhi::AllSubresources);

   protected:
    ResourceAllocator& resource_allocator_;
    ProgramVars& vars_;
    nvrhi::CommandListHandle commandList_;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE