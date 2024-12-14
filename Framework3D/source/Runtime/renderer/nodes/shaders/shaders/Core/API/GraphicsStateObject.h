/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#pragma once
#include <RCore/internal/nvrhi/nvrhi_equality.hpp>

#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/Program/ProgramVersion.h"

namespace USTC_CG {

struct GraphicsStateObjectDesc {
    static constexpr uint32_t kSampleMaskAll = -1;

    /**
     * Primitive topology
     */
    enum class PrimitiveType {
        Undefined,
        Point,
        Line,
        Triangle,
        Patch,
    };

    nvrhi::FramebufferDesc fboDesc;
    ref<const VertexLayout> pVertexLayout;
    ref<const ProgramKernels> pProgramKernels;
    ref<RasterizerState> pRasterizerState;
    ref<DepthStencilState> pDepthStencilState;
    ref<BlendState> pBlendState;
    uint32_t sampleMask = kSampleMaskAll;
    PrimitiveType primitiveType = PrimitiveType::Undefined;

    bool operator==(const GraphicsStateObjectDesc& other) const
    {
        bool result = true;
        result = result && (fboDesc == other.fboDesc);
        result = result && (pVertexLayout == other.pVertexLayout);
        result = result && (pProgramKernels == other.pProgramKernels);
        result = result && (sampleMask == other.sampleMask);
        result = result && (primitiveType == other.primitiveType);
        result = result && (pRasterizerState == other.pRasterizerState);
        result = result && (pBlendState == other.pBlendState);
        result = result && (pDepthStencilState == other.pDepthStencilState);
        return result;
    }
};

class HD_USTC_CG_API GraphicsStateObject : public Object {
    FALCOR_OBJECT(GraphicsStateObject)
   public:
    GraphicsStateObject(
        ref<Device> pDevice,
        const GraphicsStateObjectDesc& desc);
    ~GraphicsStateObject();

    nvrhi::IGraphicsPipeline* getGfxPipelineState() const
    {
        return mGfxPipelineState.Get();
    }

    const GraphicsStateObjectDesc& getDesc() const
    {
        return mDesc;
    }

    void breakStrongReferenceToDevice();

   private:
    BreakableReference<Device> mpDevice;
    GraphicsStateObjectDesc mDesc;
    nvrhi::GraphicsPipelineHandle mGfxPipelineState;

    Slang::ComPtr<nvrhi::IInputLayout> mpGFXInputLayout;

    // Default state objects
    static ref<BlendState> spDefaultBlendState;            // TODO: REMOVEGLOBAL
    static ref<RasterizerState> spDefaultRasterizerState;  // TODO: REMOVEGLOBAL
    static ref<DepthStencilState>
        spDefaultDepthStencilState;  // TODO: REMOVEGLOBAL
};
}  // namespace USTC_CG
