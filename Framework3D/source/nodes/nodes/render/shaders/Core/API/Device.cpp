/***************************************************************************
 # Copyright (c) 2015-24, NVIDIA CORPORATION. All rights reserved.
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
#include "Device.h"

#include "ComputeStateObject.h"
#include "Core/Error.h"
#include "Core/Macros.h"
#include "Core/ObjectPython.h"
#include "Core/Program/Program.h"
#include "Core/Program/ProgramManager.h"
#include "Core/Program/ShaderVar.h"
#include "GraphicsStateObject.h"
#include "RtStateObject.h"
#include "Utils/Logger.h"
#include "Utils/Scripting/ScriptBindings.h"
#include "Utils/StringUtils.h"
#include "Utils/Timing/Profiler.h"

#if FALCOR_HAS_CUDA
#include "Utils/CudaUtils.h"
#endif

#if FALCOR_HAS_D3D12
#include "Core/API/Shared/D3D12DescriptorPool.h"
#endif

#if FALCOR_NVAPI_AVAILABLE
#include <nvShaderExtnEnums.h>  // Required for checking SER support.

#include "Core/API/NvApiExDesc.h"
#endif

#include <algorithm>

#include "RenderContext.h"
#include "Utils/Logging/Logging.h"

namespace Falcor {

static_assert(getMaxViewportCount() <= 8);

static const size_t kConstantBufferDataPlacementAlignment = 256;
// This actually depends on the size of the index, but we can handle losing 2
// bytes
static const size_t kIndexBufferDataPlacementAlignment = 4;

uint64_t Device::executeCommandList(const nvrhi::CommandListHandle& commands)
{
    return getNvrhiDevice()->executeCommandList(commands);
}

nvrhi::TextureHandle Device::createTexture(
    const nvrhi::TextureDesc& texture_desc) const
{
    return mGfxDevice->createTexture(texture_desc);
}

Falcor::Device::Device(const Desc& desc) : mDesc(desc)
{
    // Create a global slang session passed to GFX and used for compiling
    // programs in ProgramManager.
    slang::createGlobalSession(mSlangGlobalSession.writeRef());

#if FALCOR_NVAPI_AVAILABLE
    mpAPIDispatcher.reset(new PipelineCreationAPIDispatcher());
    gfxDesc.apiCommandDispatcher =
        static_cast<ISlangUnknown*>(mpAPIDispatcher.get());
#endif

#if FALCOR_HAS_D3D12
    // Configure D3D12 validation layer.
    if (mDesc.type == Device::Type::D3D12 && mDesc.enableDebugLayer) {
        ID3D12Device* pD3D12Device = getNativeHandle().as<ID3D12Device*>();

        FALCOR_MAKE_SMART_COM_PTR(ID3D12InfoQueue);
        ID3D12InfoQueuePtr pInfoQueue;
        pD3D12Device->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        D3D12_MESSAGE_ID hideMessages[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
        };
        D3D12_INFO_QUEUE_FILTER f = {};
        f.DenyList.NumIDs = (UINT)std::size(hideMessages);
        f.DenyList.pIDList = hideMessages;
        pInfoQueue->AddStorageFilterEntries(&f);

        // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
        pInfoQueue->SetBreakOnID(
            D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_AT_FAULT, true);
    }
#endif

    mpProgramManager = std::make_unique<ProgramManager>(this);

    mpProfiler = std::make_unique<Profiler>(ref<Device>(this));
    mpProfiler->breakStrongReferenceToDevice();

    mpDefaultSampler = getNvrhiDevice()->createSampler(SamplerDesc{});

    mpRenderContext = std::make_unique<RenderContext>(this, mGfxCommandQueue);

    // TODO: Do we need to flush here or should RenderContext::create() bind the
    // descriptor heaps automatically without flush? See #749.
    mpRenderContext->submit();  // This will bind the descriptor heaps.
}

Device::~Device()
{
    mpRenderContext->submit(true);

    mpProfiler.reset();

    disableRaytracingValidation();

    // Release all the bound resources. Need to do that before deleting the
    // RenderContext
    mGfxCommandQueue = nullptr;
    mDeferredReleases = decltype(mDeferredReleases)();
    mpRenderContext.reset();

    mpDefaultSampler = nullptr;

#if FALCOR_HAS_D3D12
    mpD3D12CpuDescPool.reset();
    mpD3D12GpuDescPool.reset();
#endif  // FALCOR_HAS_D3D12

    mpProgramManager.reset();

    mDeferredReleases = decltype(mDeferredReleases)();

    mGfxDevice.setNull();

#if FALCOR_NVAPI_AVAILABLE
    mpAPIDispatcher.reset();
#endif
}

ref<ComputeStateObject> Device::createComputeStateObject(
    const ComputeStateObjectDesc& desc)
{
    return make_ref<ComputeStateObject>(ref<Device>(this), desc);
}

ref<GraphicsStateObject> Device::createGraphicsStateObject(
    const GraphicsStateObjectDesc& desc)
{
    return make_ref<GraphicsStateObject>(ref<Device>(this), desc);
}

ref<RtStateObject> Device::createRtStateObject(const RtStateObjectDesc& desc)
{
    return make_ref<RtStateObject>(ref<Device>(this), desc);
}

void Device::wait()
{
    mpRenderContext->submit(true);
    // mpRenderContext->signal(mpFrameFence.get());
    executeDeferredReleases();
}

void Device::endFrame()
{
    mpRenderContext->submit();

    USTC_CG::logging("Endframe needs refactoring");

    //// Wait on past frames.
    // if (mpFrameFence->getSignaledValue() > kInFlightFrameCount)
    //     mpFrameFence->wait(
    //         mpFrameFence->getSignaledValue() - kInFlightFrameCount);

    //// Flush ray tracing validation if enabled
    // flushRaytracingValidation();

    //// Switch to next transient resource heap.
    // getCurrentTransientResourceHeap()->finish();
    // mCurrentTransientResourceHeapIndex =
    //     (mCurrentTransientResourceHeapIndex + 1) % kInFlightFrameCount;
    // mpRenderContext->getCommandList()->closeCommandBuffer();
    // getCurrentTransientResourceHeap()->synchronizeAndReset();
    // mpRenderContext->getCommandList()->openCommandBuffer();

    //// Signal frame fence for new frame.
    // mpRenderContext->signal(mpFrameFence.get());

    // Release resources from past frames.
    executeDeferredReleases();
}

}  // namespace Falcor
