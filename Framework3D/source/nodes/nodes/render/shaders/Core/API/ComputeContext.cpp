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
#include "ComputeContext.h"

#include "Core/Program/ProgramVars.h"
#include "Core/State/ComputeState.h"
#include "Utils/Logging/Logging.h"

namespace Falcor {
ComputeContext::ComputeContext(Device* pDevice, nvrhi::ICommandList* pQueue)
    : CopyContext(pDevice, pQueue)
{
}

ComputeContext::~ComputeContext()
{
}

void ComputeContext::dispatch(
    ComputeState* pState,
    ProgramVars* pVars,
    const uint3& dispatchSize)
{
    pVars->prepareDescriptorSets(this);

    auto computeEncoder = mpLowLevelData;

    nvrhi::ComputeState state;
    state.pipeline = pState->getCSO(pVars)->getGfxPipelineState();
    state.bindings = pVars->getBindings();

    computeEncoder->setComputeState(state);
    computeEncoder->dispatch(
        (int)dispatchSize.x, (int)dispatchSize.y, (int)dispatchSize.z);
    mCommandsPending = true;
}

void ComputeContext::dispatchIndirect(
    ComputeState* pState,
    ProgramVars* pVars,
    Buffer* pArgBuffer,
    uint64_t argBufferOffset)
{
    pVars->prepareDescriptorSets(this);
    resourceBarrier(pArgBuffer, ResourceStates::IndirectArgument);

    auto computeEncoder = mpLowLevelData;

    nvrhi::ComputeState state;
    state.pipeline = pState->getCSO(pVars)->getGfxPipelineState();
    state.bindings = pVars->getBindings();
    state.indirectParams = pArgBuffer;

    computeEncoder->setComputeState(state);

    computeEncoder->dispatchIndirect(argBufferOffset);
    mCommandsPending = true;
}

void ComputeContext::clearUAV(
    const UnorderedAccessView* pUav,
    const float4& value)
{
    resourceBarrier(pUav->resourceHandle, ResourceStates::UnorderedAccess);
    
    auto resourceEncoder = mpLowLevelData;
    ClearValue clearValue = {};
    memcpy(&clearValue, &value, sizeof(float) * 4);
    resourceEncoder->clearBufferUInt() clearResourceView(
        pUav->getGfxResourceView(),
        &clearValue,
        ClearResourceViewFlags::FloatClearValues);
    mCommandsPending = true;
}

void ComputeContext::clearUAV(
    const UnorderedAccessView* pUav,
    const uint4& value)
{
    resourceBarrier(pUav->resourceHandle, ResourceStates::UnorderedAccess);

    auto resourceEncoder = mpLowLevelData;
    ClearValue clearValue = {};
    memcpy(clearValue.color.uintValues, &value, sizeof(uint32_t) * 4);
    
    resourceEncoder->clearResourceView(
        pUav->getGfxResourceView(), &clearValue, ClearResourceViewFlags::None);
    mCommandsPending = true;
}

void ComputeContext::clearUAVCounter(
    const nvrhi::BufferHandle& pBuffer,
    uint32_t value)
{
    USTC_CG::logging("counter UAV WIP");
    // if (pBuffer->getUAVCounter()) {
    //     resourceBarrier(
    //         pBuffer->getUAVCounter().get(),
    //         Resource::State::UnorderedAccess);

    //    auto resourceEncoder = mpLowLevelData;
    //    ClearValue clearValue = {};
    //    clearValue.color.uintValues[0] = clearValue.color.uintValues[1] =
    //        clearValue.color.uintValues[2] = clearValue.color.uintValues[3] =
    //            value;
    //    resourceEncoder->clearResourceView(
    //        pBuffer->getUAVCounter()->getUAV()->getGfxResourceView(),
    //        &clearValue,
    //        ClearResourceViewFlags::None);
    //    mCommandsPending = true;
    //}
}

void ComputeContext::submit(bool wait)
{
    CopyContext::submit(wait);
    mpLastBoundComputeVars = nullptr;
}
}  // namespace Falcor
