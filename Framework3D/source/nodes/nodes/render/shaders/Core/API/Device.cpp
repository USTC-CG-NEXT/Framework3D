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
namespace Falcor {
static_assert(sizeof(AdapterLUID) == sizeof(nvrhi::AdapterLUID));

static_assert((uint32_t)RayFlags::None == 0);
static_assert((uint32_t)RayFlags::ForceOpaque == 0x1);
static_assert((uint32_t)RayFlags::ForceNonOpaque == 0x2);
static_assert((uint32_t)RayFlags::AcceptFirstHitAndEndSearch == 0x4);
static_assert((uint32_t)RayFlags::SkipClosestHitShader == 0x8);
static_assert((uint32_t)RayFlags::CullBackFacingTriangles == 0x10);
static_assert((uint32_t)RayFlags::CullFrontFacingTriangles == 0x20);
static_assert((uint32_t)RayFlags::CullOpaque == 0x40);
static_assert((uint32_t)RayFlags::CullNonOpaque == 0x80);
static_assert((uint32_t)RayFlags::SkipTriangles == 0x100);
static_assert((uint32_t)RayFlags::SkipProceduralPrimitives == 0x200);

static_assert(getMaxViewportCount() <= 8);

static const uint32_t kTransientHeapConstantBufferSize = 16 * 1024 * 1024;

static const size_t kConstantBufferDataPlacementAlignment = 256;
// This actually depends on the size of the index, but we can handle losing 2
// bytes
static const size_t kIndexBufferDataPlacementAlignment = 4;

/// The default Shader Model to use when compiling programs.
/// If not supported, the highest supported shader model will be used instead.
static const ShaderModel kDefaultShaderModel = ShaderModel::SM6_6;

class GFXDebugCallBack : public nvrhi::IDebugCallback {
    virtual SLANG_NO_THROW void SLANG_MCALL handleMessage(
        nvrhi::DebugMessageType type,
        nvrhi::DebugMessageSource source,
        const char* message) override
    {
        if (type == nvrhi::DebugMessageType::Error) {
            logError("GFX Error: {}", message);
        }
        else if (type == nvrhi::DebugMessageType::Warning) {
            logWarning("GFX Warning: {}", message);
        }
        else {
            logDebug("GFX Info: {}", message);
        }
    }
};

GFXDebugCallBack gGFXDebugCallBack;  // TODO: REMOVEGLOBAL

#if FALCOR_NVAPI_AVAILABLE
// To use NVAPI, we intercept the API calls in the nvrhi layer and dispatch into
// the NVAPI_Create*PipelineState functions instead if the shader uses NVAPI
// functionalities. We use the nvrhi API dispatcher mechanism to intercept and
// redirect the API call. This is done by defining an implementation of
// `IPipelineCreationAPIDispatcher` and passing an instance of this
// implementation to `gfxCreateDevice`.
class PipelineCreationAPIDispatcher
    : public nvrhi::IPipelineCreationAPIDispatcher {
   private:
    bool findNvApiShaderParameter(
        slang::IComponentType* program,
        uint32_t& space,
        uint32_t& registerId)
    {
        auto globalTypeLayout =
            program->getLayout()->getGlobalParamsVarLayout()->getTypeLayout();
        auto index = globalTypeLayout->findFieldIndexByName("g_NvidiaExt");
        if (index != -1) {
            auto field = globalTypeLayout->getFieldByIndex((unsigned int)index);
            space = field->getBindingSpace();
            registerId = field->getBindingIndex();
            return true;
        }
        return false;
    }

    void createNvApiUavSlotExDesc(
        NvApiPsoExDesc& ret,
        uint32_t space,
        uint32_t uavSlot)
    {
        ret.psoExtension = NV_PSO_SET_SHADER_EXTNENSION_SLOT_AND_SPACE;

        auto& desc = ret.mExtSlotDesc;
        std::memset(&desc, 0, sizeof(desc));

        desc.psoExtension = NV_PSO_SET_SHADER_EXTNENSION_SLOT_AND_SPACE;
        desc.version = NV_SET_SHADER_EXTENSION_SLOT_DESC_VER;
        desc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
        desc.uavSlot = uavSlot;
        desc.registerSpace = space;
    }

   public:
    PipelineCreationAPIDispatcher()
    {
        if (NvAPI_Initialize() != NVAPI_OK) {
            FALCOR_THROW("Failed to initialize NVAPI.");
        }
    }

    ~PipelineCreationAPIDispatcher()
    {
        NvAPI_Unload();
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) override
    {
        if (uuid == SlangUUID SLANG_UUID_IPipelineCreationAPIDispatcher) {
            *outObject =
                static_cast<nvrhi::IPipelineCreationAPIDispatcher*>(this);
            return SLANG_OK;
        }
        return SLANG_E_NO_INTERFACE;
    }

    // The lifetime of this dispatcher object will be managed by
    // `Falcor::Device` so we don't need to actually implement reference
    // counting here.
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override
    {
        return 2;
    }

    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override
    {
        // Returning 2 is important here, because when releasing a COM pointer,
        // it checks if the ref count **was 1 before releasing** in order to
        // free the object.
        return 2;
    }

    // This method will be called by the nvrhi layer to create an API object for
    // a compute pipeline state.
    virtual nvrhi::Result createComputePipelineState(
        nvrhi::IDevice* device,
        slang::IComponentType* program,
        void* pipelineDesc,
        void** outPipelineState)
    {
        nvrhi::IDevice::InteropHandles nativeHandle;
        FALCOR_GFX_CALL(device->getNativeDeviceHandles(&nativeHandle));
        ID3D12Device* pD3D12Device = reinterpret_cast<ID3D12Device*>(
            nativeHandle.handles[0].handleValue);

        uint32_t space, registerId;
        if (findNvApiShaderParameter(program, space, registerId)) {
            NvApiPsoExDesc psoDesc = {};
            createNvApiUavSlotExDesc(psoDesc, space, registerId);
            const NVAPI_D3D12_PSO_EXTENSION_DESC* ppPSOExtensionsDesc[1] = {
                &psoDesc.mExtSlotDesc
            };
            auto result = NvAPI_D3D12_CreateComputePipelineState(
                pD3D12Device,
                reinterpret_cast<D3D12_COMPUTE_PIPELINE_STATE_DESC*>(
                    pipelineDesc),
                1,
                ppPSOExtensionsDesc,
                (ID3D12PipelineState**)outPipelineState);
            return (result == NVAPI_OK) ? SLANG_OK : SLANG_FAIL;
        }
        else {
            ID3D12PipelineState* pState = nullptr;
            SLANG_RETURN_ON_FAIL(pD3D12Device->CreateComputePipelineState(
                reinterpret_cast<D3D12_COMPUTE_PIPELINE_STATE_DESC*>(
                    pipelineDesc),
                IID_PPV_ARGS(&pState)));
            *outPipelineState = pState;
        }
        return SLANG_OK;
    }

    // This method will be called by the nvrhi layer to create an API object for
    // a graphics pipeline state.
    virtual nvrhi::Result createGraphicsPipelineState(
        nvrhi::IDevice* device,
        slang::IComponentType* program,
        void* pipelineDesc,
        void** outPipelineState)
    {
        nvrhi::IDevice::InteropHandles nativeHandle;
        FALCOR_GFX_CALL(device->getNativeDeviceHandles(&nativeHandle));
        ID3D12Device* pD3D12Device = reinterpret_cast<ID3D12Device*>(
            nativeHandle.handles[0].handleValue);

        uint32_t space, registerId;
        if (findNvApiShaderParameter(program, space, registerId)) {
            NvApiPsoExDesc psoDesc = {};
            createNvApiUavSlotExDesc(psoDesc, space, registerId);
            const NVAPI_D3D12_PSO_EXTENSION_DESC* ppPSOExtensionsDesc[1] = {
                &psoDesc.mExtSlotDesc
            };

            auto result = NvAPI_D3D12_CreateGraphicsPipelineState(
                pD3D12Device,
                reinterpret_cast<D3D12_GRAPHICS_PIPELINE_STATE_DESC*>(
                    pipelineDesc),
                1,
                ppPSOExtensionsDesc,
                (ID3D12PipelineState**)outPipelineState);
            return (result == NVAPI_OK) ? SLANG_OK : SLANG_FAIL;
        }
        else {
            ID3D12PipelineState* pState = nullptr;
            SLANG_RETURN_ON_FAIL(pD3D12Device->CreateGraphicsPipelineState(
                reinterpret_cast<D3D12_GRAPHICS_PIPELINE_STATE_DESC*>(
                    pipelineDesc),
                IID_PPV_ARGS(&pState)));
            *outPipelineState = pState;
        }
        return SLANG_OK;
    }

    virtual nvrhi::Result createMeshPipelineState(
        nvrhi::IDevice* device,
        slang::IComponentType* program,
        void* pipelineDesc,
        void** outPipelineState)
    {
        FALCOR_THROW("Mesh pipelines are not supported.");
    }

    // This method will be called by the nvrhi layer right before creating a ray
    // tracing state object.
    virtual nvrhi::Result beforeCreateRayTracingState(
        nvrhi::IDevice* device,
        slang::IComponentType* program)
    {
        nvrhi::IDevice::InteropHandles nativeHandle;
        FALCOR_GFX_CALL(device->getNativeDeviceHandles(&nativeHandle));
        ID3D12Device* pD3D12Device = reinterpret_cast<ID3D12Device*>(
            nativeHandle.handles[0].handleValue);

        uint32_t space, registerId;
        if (findNvApiShaderParameter(program, space, registerId)) {
            if (NvAPI_D3D12_SetNvShaderExtnSlotSpace(
                    pD3D12Device, registerId, space) != NVAPI_OK) {
                FALCOR_THROW("Failed to set NvApi extension");
            }
        }

        return SLANG_OK;
    }

    // This method will be called by the nvrhi layer right after creating a ray
    // tracing state object.
    virtual nvrhi::Result afterCreateRayTracingState(
        nvrhi::IDevice* device,
        slang::IComponentType* program)
    {
        nvrhi::IDevice::InteropHandles nativeHandle;
        FALCOR_GFX_CALL(device->getNativeDeviceHandles(&nativeHandle));
        ID3D12Device* pD3D12Device = reinterpret_cast<ID3D12Device*>(
            nativeHandle.handles[0].handleValue);

        uint32_t space, registerId;
        if (findNvApiShaderParameter(program, space, registerId)) {
            if (NvAPI_D3D12_SetNvShaderExtnSlotSpace(
                    pD3D12Device, 0xFFFFFFFF, 0) != NVAPI_OK) {
                FALCOR_THROW("Failed to set NvApi extension");
            }
        }
        return SLANG_OK;
    }
};
#endif  // FALCOR_NVAPI_AVAILABLE

inline Device::Type getDefaultDeviceType()
{
#if FALCOR_HAS_D3D12
    return Device::Type::D3D12;
#elif FALCOR_HAS_VULKAN
    return Device::Type::Vulkan;
#else
    FALCOR_THROW("No default device type");
#endif
}

inline nvrhi::DeviceType getGfxDeviceType(Device::Type deviceType)
{
    switch (deviceType) {
        case Device::Type::Default: return nvrhi::DeviceType::Default;
        case Device::Type::D3D12: return nvrhi::DeviceType::DirectX12;
        case Device::Type::Vulkan: return nvrhi::DeviceType::Vulkan;
        default: FALCOR_THROW("Unknown device type");
    }
}

inline Device::Limits queryLimits(nvrhi::IDevice* pDevice)
{
    const auto& deviceLimits = pDevice->getDeviceInfo().limits;

    auto toUint3 = [](const uint32_t value[]) {
        return uint3(value[0], value[1], value[2]);
    };

    Device::Limits limits = {};
    limits.maxComputeDispatchThreadGroups =
        toUint3(deviceLimits.maxComputeDispatchThreadGroups);
    limits.maxShaderVisibleSamplers = deviceLimits.maxShaderVisibleSamplers;
    return limits;
}

uint64_t Device::executeCommandList(const nvrhi::CommandListHandle& commands)
{
    return getGfxDevice()->executeCommandList(commands);
}

Falcor::Device::Device(const Desc& desc) : mDesc(desc)
{
    if (mDesc.enableAftermath) {
#if FALCOR_HAS_AFTERMATH
        // Aftermath is incompatible with debug layers, so lets disable them.
        mDesc.enableDebugLayer = false;
        enableAftermath();
#else
        logWarning(
            "Falcor was compiled without Aftermath support. Aftermath is "
            "disabled");
#endif
    }

    // Create a global slang session passed to GFX and used for compiling
    // programs in ProgramManager.
    slang::createGlobalSession(mSlangGlobalSession.writeRef());

    if (mDesc.type == Type::Default)
        mDesc.type = getDefaultDeviceType();

#if !FALCOR_HAS_D3D12
    if (mDesc.type == Type::D3D12)
        FALCOR_THROW("D3D12 device not supported.");
#endif
#if !FALCOR_HAS_VULKAN
    if (mDesc.type == Type::Vulkan)
        FALCOR_THROW("Vulkan device not supported.");
#endif

    nvrhi::IDevice::Desc gfxDesc = {};
    gfxDesc.deviceType = getGfxDeviceType(mDesc.type);
    gfxDesc.slang.slangGlobalSession = mSlangGlobalSession;

    // Setup shader cache.
    gfxDesc.shaderCache.maxEntryCount = mDesc.maxShaderCacheEntryCount;
    if (mDesc.shaderCachePath == "") {
        gfxDesc.shaderCache.shaderCachePath = nullptr;
    }
    else {
        gfxDesc.shaderCache.shaderCachePath = mDesc.shaderCachePath.c_str();
        // If the supplied shader cache path does not exist, we will need to
        // create it before creating the device.
        if (std::filesystem::exists(mDesc.shaderCachePath)) {
            if (!std::filesystem::is_directory(mDesc.shaderCachePath))
                FALCOR_THROW(
                    "Shader cache path {} exists and is not a directory",
                    mDesc.shaderCachePath);
        }
        else {
            std::filesystem::create_directories(mDesc.shaderCachePath);
        }
    }

    std::vector<void*> extendedDescs;
    // Add extended desc for root parameter attribute.
    nvrhi::D3D12DeviceExtendedDesc extDesc = {};
    extDesc.rootParameterShaderAttributeName = "root";
    extendedDescs.push_back(&extDesc);
#if FALCOR_HAS_D3D12
    // Add extended descs for experimental API features.
    nvrhi::D3D12ExperimentalFeaturesDesc experimentalFeaturesDesc = {};
    experimentalFeaturesDesc.numFeatures =
        (uint32_t)mDesc.experimentalFeatures.size();
    experimentalFeaturesDesc.featureIIDs = mDesc.experimentalFeatures.data();
    if (gfxDesc.deviceType == nvrhi::DeviceType::DirectX12)
        extendedDescs.push_back(&experimentalFeaturesDesc);
#endif
    gfxDesc.extendedDescCount = extendedDescs.size();
    gfxDesc.extendedDescs = extendedDescs.data();

#if FALCOR_NVAPI_AVAILABLE
    mpAPIDispatcher.reset(new PipelineCreationAPIDispatcher());
    gfxDesc.apiCommandDispatcher =
        static_cast<ISlangUnknown*>(mpAPIDispatcher.get());
#endif

    // Setup debug layer.
    FALCOR_GFX_CALL(gfxSetDebugCallback(&gGFXDebugCallBack));
    if (mDesc.enableDebugLayer)
        nvrhi::gfxEnableDebugLayer();

    // Get list of available GPUs.
    const auto gpus = getGPUs(mDesc.type);

    if (gpus.size() == 0) {
        FALCOR_THROW(
            "Did not find any GPUs for device type '{}'.",
            enumToString<decltype(mDesc.type)>(mDesc.type));
    }

    if (mDesc.gpu >= gpus.size()) {
        logWarning(
            "GPU index {} is out of range, using first GPU instead.",
            mDesc.gpu);
        mDesc.gpu = 0;
    }

    // Try to create device on specific GPU.
    {
        gfxDesc.adapterLUID =
            reinterpret_cast<const nvrhi::AdapterLUID*>(&gpus[mDesc.gpu].luid);
        if (SLANG_FAILED(gfxCreateDevice(&gfxDesc, mGfxDevice.writeRef())))
            logWarning(
                "Failed to create device on GPU {} ({}).",
                mDesc.gpu,
                gpus[mDesc.gpu].name);
    }

    // Otherwise try create device on any available GPU.
    if (!mGfxDevice) {
        gfxDesc.adapterLUID = nullptr;
        if (SLANG_FAILED(gfxCreateDevice(&gfxDesc, mGfxDevice.writeRef())))
            FALCOR_THROW("Failed to create device");
    }

    const auto& deviceInfo = mGfxDevice->getDeviceInfo();
    mInfo.adapterName = deviceInfo.adapterName;
    mInfo.adapterLUID =
        gfxDesc.adapterLUID ? gpus[mDesc.gpu].luid : AdapterLUID();
    mInfo.apiName = deviceInfo.apiName;
    mLimits = queryLimits(mGfxDevice);
    mSupportedFeatures = querySupportedFeatures(mGfxDevice);

    // Attempt to enable ray tracing validation if requested
    if (mDesc.enableRaytracingValidation)
        enableRaytracingValidation();

#if FALCOR_HAS_AFTERMATH
    if (mDesc.enableAftermath) {
        mpAftermathContext = std::make_unique<AftermathContext>(this);
        mpAftermathContext->initialize();
    }
#endif

#if FALCOR_NVAPI_AVAILABLE
    // Explicitly check for SER support via NVAPI.
    // Slang currently relies on NVAPI to implement the SER API but cannot check
    // it's availibility due to not being shipped with NVAPI for licensing
    // reasons.
    if (getType() == Type::D3D12) {
        ID3D12Device* pD3D12Device = getNativeHandle().as<ID3D12Device*>();
        // First check for avalibility of SER API (HitObject).
        bool supportSER = false;
        NvAPI_Status ret = NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(
            pD3D12Device, NV_EXTN_OP_HIT_OBJECT_REORDER_THREAD, &supportSER);
        if (ret == NVAPI_OK && supportSER)
            mSupportedFeatures |=
                SupportedFeatures::ShaderExecutionReorderingAPI;

        // Then check for hardware support.
        NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS reorderingCaps;
        ret = NvAPI_D3D12_GetRaytracingCaps(
            pD3D12Device,
            NVAPI_D3D12_RAYTRACING_CAPS_TYPE_THREAD_REORDERING,
            &reorderingCaps,
            sizeof(reorderingCaps));
        if (ret == NVAPI_OK &&
            reorderingCaps ==
                NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAP_STANDARD)
            mSupportedFeatures |= SupportedFeatures::RaytracingReordering;
    }
#endif
    if (getType() == Type::Vulkan) {
        // Vulkan always supports SER.
        mSupportedFeatures |= SupportedFeatures::ShaderExecutionReorderingAPI;
    }

    mSupportedShaderModel = querySupportedShaderModel(mGfxDevice);
    mDefaultShaderModel = std::min(kDefaultShaderModel, mSupportedShaderModel);
    mGpuTimestampFrequency =
        1000.0 / (double)mGfxDevice->getDeviceInfo().timestampFrequency;

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

    for (uint32_t i = 0; i < kInFlightFrameCount; ++i) {
        nvrhi::ITransientResourceHeap::Desc transientHeapDesc = {};
        transientHeapDesc.flags =
            nvrhi::ITransientResourceHeap::Flags::AllowResizing;
        transientHeapDesc.constantBufferSize = kTransientHeapConstantBufferSize;
        transientHeapDesc.samplerDescriptorCount = 2048;
        transientHeapDesc.uavDescriptorCount = 1000000;
        transientHeapDesc.srvDescriptorCount = 1000000;
        transientHeapDesc.constantBufferDescriptorCount = 1000000;
        transientHeapDesc.accelerationStructureDescriptorCount = 1000000;
        if (SLANG_FAILED(mGfxDevice->createTransientResourceHeap(
                transientHeapDesc, mpTransientResourceHeaps[i].writeRef())))
            FALCOR_THROW("Failed to create transient resource heap");
    }

    nvrhi::nvrhi::ICommandList::Desc queueDesc = {};
    queueDesc.type = nvrhi::nvrhi::ICommandList::QueueType::Graphics;
    if (SLANG_FAILED(mGfxDevice->createCommandQueue(
            queueDesc, mGfxCommandQueue.writeRef())))
        FALCOR_THROW("Failed to create command queue");

    // The Device class contains a bunch of nested resource objects that have
    // strong references to the device. This is because we want a strong
    // reference to the device when those objects are returned to the user.
    // However, here it immediately creates cyclic references
    // device->resource->device upon creation of the device. To break the
    // cycles, we break the strong reference to the device for the resources
    // that it owns.

    // Here, we temporarily increase the refcount of the device, so it won't be
    // destroyed upon breaking the nested strong references to it.
    this->incRef();

#if FALCOR_ENABLE_REF_TRACKING
    this->setEnableRefTracking(true);
#endif

    mpFrameFence = createFence();
    mpFrameFence->breakStrongReferenceToDevice();

#if FALCOR_HAS_D3D12
    if (getType() == Type::D3D12) {
        // Create the descriptor pools
        D3D12DescriptorPool::Desc poolDesc;
        poolDesc.setDescCount(ResourceType::TextureSrv, 1000000)
            .setDescCount(ResourceType::Sampler, 2048)
            .setShaderVisible(true);
        mpD3D12GpuDescPool =
            D3D12DescriptorPool::create(this, poolDesc, mpFrameFence);
        poolDesc.setShaderVisible(false)
            .setDescCount(ResourceType::Rtv, 16 * 1024)
            .setDescCount(ResourceType::Dsv, 1024);
        mpD3D12CpuDescPool =
            D3D12DescriptorPool::create(this, poolDesc, mpFrameFence);
    }
#endif  // FALCOR_HAS_D3D12

    mpProgramManager = std::make_unique<ProgramManager>(this);

    mpProfiler = std::make_unique<Profiler>(ref<Device>(this));
    mpProfiler->breakStrongReferenceToDevice();

    mpDefaultSampler = createSampler(Sampler::Desc());
    mpDefaultSampler->breakStrongReferenceToDevice();

    mpUploadHeap = GpuMemoryHeap::create(
        ref<Device>(this), MemoryType::Upload, 1024 * 1024 * 2, mpFrameFence);
    mpUploadHeap->breakStrongReferenceToDevice();

    mpReadBackHeap = GpuMemoryHeap::create(
        ref<Device>(this), MemoryType::ReadBack, 1024 * 1024 * 2, mpFrameFence);
    mpReadBackHeap->breakStrongReferenceToDevice();

    mpTimestampQueryHeap = QueryHeap::create(
        ref<Device>(this), QueryHeap::Type::Timestamp, 1024 * 1024);
    mpTimestampQueryHeap->breakStrongReferenceToDevice();

    mpRenderContext = std::make_unique<RenderContext>(this, mGfxCommandQueue);

    // TODO: Do we need to flush here or should RenderContext::create() bind the
    // descriptor heaps automatically without flush? See #749.
    mpRenderContext->submit();  // This will bind the descriptor heaps.

    this->decRef(false);

    logInfo(
        "Created GPU device '{}' using '{}' API (SM{}.{}).",
        mInfo.adapterName,
        mInfo.apiName,
        getShaderModelMajorVersion(mSupportedShaderModel),
        getShaderModelMinorVersion(mSupportedShaderModel));
}

Device::~Device()
{
    mpRenderContext->submit(true);

    mpProfiler.reset();

    disableRaytracingValidation();

    // Release all the bound resources. Need to do that before deleting the
    // RenderContext
    mGfxCommandQueue.setNull();
    mDeferredReleases = decltype(mDeferredReleases)();
    mpRenderContext.reset();
    mpUploadHeap.reset();
    mpReadBackHeap.reset();
    mpTimestampQueryHeap.reset();
    for (size_t i = 0; i < kInFlightFrameCount; ++i)
        mpTransientResourceHeaps[i].setNull();

    mpDefaultSampler.reset();
    mpFrameFence.reset();

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

size_t Device::getBufferDataAlignment(ResourceBindFlags bindFlags)
{
    if (is_set(bindFlags, ResourceBindFlags::Constant))
        return kConstantBufferDataPlacementAlignment;
    if (is_set(bindFlags, ResourceBindFlags::Index))
        return kIndexBufferDataPlacementAlignment;
    return 1;
}

void Device::releaseResource(ISlangUnknown* pResource)
{
    if (pResource) {
        // Some static objects get here when the application exits
        if (this) {
            mDeferredReleases.push(
                { mpFrameFence ? mpFrameFence->getSignaledValue() : 0,
                  Slang::ComPtr<ISlangUnknown>(pResource) });
        }
    }
}

bool Device::isFeatureSupported(SupportedFeatures flags) const
{
    return is_set(mSupportedFeatures, flags);
}

bool Device::isShaderModelSupported(ShaderModel shaderModel) const
{
    return ((uint32_t)shaderModel <= (uint32_t)mSupportedShaderModel);
}

void Device::executeDeferredReleases()
{
    mpUploadHeap->executeDeferredReleases();
    mpReadBackHeap->executeDeferredReleases();
    uint64_t currentValue = mpFrameFence->getCurrentValue();
    while (mDeferredReleases.size() &&
           mDeferredReleases.front().fenceValue < currentValue) {
        mDeferredReleases.pop();
    }

#if FALCOR_HAS_D3D12
    if (getType() == Type::D3D12) {
        mpD3D12CpuDescPool->executeDeferredReleases();
        mpD3D12GpuDescPool->executeDeferredReleases();
    }
#endif  // FALCOR_HAS_D3D12
}

void Device::wait()
{
    mpRenderContext->submit(true);
    mpRenderContext->signal(mpFrameFence.get());
    executeDeferredReleases();
}

void Device::requireD3D12() const
{
    if (getType() != Type::D3D12)
        FALCOR_THROW("D3D12 device is required.");
}

void Device::requireVulkan() const
{
    if (getType() != Type::Vulkan)
        FALCOR_THROW("Vulkan device is required.");
}

size_t Device::getTextureRowAlignment() const
{
    size_t alignment = 1;
    mGfxDevice->getTextureRowAlignment(&alignment);
    return alignment;
}

#if FALCOR_HAS_CUDA

bool Device::initCudaDevice()
{
    return getCudaDevice() != nullptr;
}

cuda_utils::CudaDevice* Device::getCudaDevice() const
{
    if (!mpCudaDevice)
        mpCudaDevice = make_ref<cuda_utils::CudaDevice>(this);
    return mpCudaDevice.get();
}

#endif

bool Device::enableAgilitySDK()
{
#if FALCOR_WINDOWS && FALCOR_HAS_D3D12 && FALCOR_HAS_D3D12_AGILITY_SDK
    std::filesystem::path exeDir = getExecutableDirectory();
    std::filesystem::path sdkDir =
        getRuntimeDirectory() / FALCOR_D3D12_AGILITY_SDK_PATH;

    // Agility SDK can only be loaded from a relative path to the executable.
    // Make sure both paths use the same driver letter.
    if (std::tolower(exeDir.string()[0]) != std::tolower(sdkDir.string()[0])) {
        logWarning(
            "Cannot enable D3D12 Agility SDK: Executable directory '{}' is not "
            "on the same drive as the SDK directory '{}'.",
            exeDir,
            sdkDir);
        return false;
    }

    // Get relative path and make sure there is the required trailing path
    // delimiter.
    auto relPath = std::filesystem::relative(sdkDir, exeDir) / "";

    // Get the D3D12GetInterface procedure.
    typedef HRESULT(WINAPI * D3D12GetInterfaceFn)(
        REFCLSID rclsid, REFIID riid, void** ppvDebug);
    HMODULE handle = GetModuleHandleA("d3d12.dll");
    D3D12GetInterfaceFn pD3D12GetInterface =
        handle
            ? (D3D12GetInterfaceFn)GetProcAddress(handle, "D3D12GetInterface")
            : nullptr;
    if (!pD3D12GetInterface) {
        logWarning(
            "Cannot enable D3D12 Agility SDK: Failed to get "
            "D3D12GetInterface.");
        return false;
    }

    // Local definition of CLSID_D3D12SDKConfiguration from d3d12.h
    const GUID CLSID_D3D12SDKConfiguration__ = {
        0x7cda6aca,
        0xa03e,
        0x49c8,
        { 0x94, 0x58, 0x03, 0x34, 0xd2, 0x0e, 0x07, 0xce }
    };
    // Get the D3D12SDKConfiguration interface.
    FALCOR_MAKE_SMART_COM_PTR(ID3D12SDKConfiguration);
    ID3D12SDKConfigurationPtr pD3D12SDKConfiguration;
    if (!SUCCEEDED(pD3D12GetInterface(
            CLSID_D3D12SDKConfiguration__,
            IID_PPV_ARGS(&pD3D12SDKConfiguration)))) {
        logWarning(
            "Cannot enable D3D12 Agility SDK: Failed to get "
            "D3D12SDKConfiguration interface.");
        return false;
    }

    // Set the SDK version and path.
    if (!SUCCEEDED(pD3D12SDKConfiguration->SetSDKVersion(
            FALCOR_D3D12_AGILITY_SDK_VERSION, relPath.string().c_str()))) {
        logWarning(
            "Cannot enable D3D12 Agility SDK: Calling SetSDKVersion failed.");
        return false;
    }

    return true;
#endif
    return false;
}

std::vector<AdapterInfo> Device::getGPUs(Type deviceType)
{
    if (deviceType == Type::Default)
        deviceType = getDefaultDeviceType();
    auto adapters = nvrhi::gfxGetAdapters(getGfxDeviceType(deviceType));
    std::vector<AdapterInfo> result;
    for (nvrhi::GfxIndex i = 0; i < adapters.getCount(); ++i) {
        const nvrhi::AdapterInfo& gfxInfo = adapters.getAdapters()[i];
        AdapterInfo info;
        info.name = gfxInfo.name;
        info.vendorID = gfxInfo.vendorID;
        info.deviceID = gfxInfo.deviceID;
        info.luid = *reinterpret_cast<const AdapterLUID*>(&gfxInfo.luid);
        result.push_back(info);
    }
    // Move all NVIDIA adapters to the start of the list.
    std::stable_partition(
        result.begin(), result.end(), [](const AdapterInfo& info) {
            return toLowerCase(info.name).find("nvidia") != std::string::npos;
        });
    return result;
}

nvrhi::ITransientResourceHeap* Device::getCurrentTransientResourceHeap()
{
    return mpTransientResourceHeaps[mCurrentTransientResourceHeapIndex].get();
}

void Device::endFrame()
{
    mpRenderContext->submit();

    // Wait on past frames.
    if (mpFrameFence->getSignaledValue() > kInFlightFrameCount)
        mpFrameFence->wait(
            mpFrameFence->getSignaledValue() - kInFlightFrameCount);

    // Flush ray tracing validation if enabled
    flushRaytracingValidation();

    // Switch to next transient resource heap.
    getCurrentTransientResourceHeap()->finish();
    mCurrentTransientResourceHeapIndex =
        (mCurrentTransientResourceHeapIndex + 1) % kInFlightFrameCount;
    mpRenderContext->getLowLevelData()->closeCommandBuffer();
    getCurrentTransientResourceHeap()->synchronizeAndReset();
    mpRenderContext->getLowLevelData()->openCommandBuffer();

    // Signal frame fence for new frame.
    mpRenderContext->signal(mpFrameFence.get());

    // Release resources from past frames.
    executeDeferredReleases();
}

NativeHandle Device::getNativeHandle(uint32_t index) const
{
    nvrhi::IDevice::InteropHandles gfxInteropHandles = {};
    FALCOR_GFX_CALL(mGfxDevice->getNativeDeviceHandles(&gfxInteropHandles));

#if FALCOR_HAS_D3D12
    if (getType() == Device::Type::D3D12) {
        if (index == 0)
            return NativeHandle(reinterpret_cast<ID3D12Device*>(
                gfxInteropHandles.handles[0].handleValue));
    }
#endif
#if FALCOR_HAS_VULKAN
    if (getType() == Device::Type::Vulkan) {
        if (index == 0)
            return NativeHandle(reinterpret_cast<VkInstance>(
                gfxInteropHandles.handles[0].handleValue));
        else if (index == 1)
            return NativeHandle(reinterpret_cast<VkPhysicalDevice>(
                gfxInteropHandles.handles[1].handleValue));
        else if (index == 2)
            return NativeHandle(reinterpret_cast<VkDevice>(
                gfxInteropHandles.handles[2].handleValue));
    }
#endif
    return {};
}

// Debug log message from ray tracing validation system
#if FALCOR_NVAPI_AVAILABLE && FALCOR_HAS_D3D12
static void RaytracingValidationCallback(
    void* pUserData,
    NVAPI_D3D12_RAYTRACING_VALIDATION_MESSAGE_SEVERITY severity,
    const char* messageCode,
    const char* message,
    const char* messageDetails)
{
    if (severity == NVAPI_D3D12_RAYTRACING_VALIDATION_MESSAGE_SEVERITY_ERROR)
        logError(
            "Raytracing validation error {}:\n{}\n{}",
            messageCode,
            message,
            messageDetails);
    else
        logWarning(
            "Raytracing validation warning {}:\n{}\n{}",
            messageCode,
            message,
            messageDetails);
}
#endif

void Device::enableRaytracingValidation()
{
#if FALCOR_NVAPI_AVAILABLE
    if (!isFeatureSupported(Device::SupportedFeatures::Raytracing))
        FALCOR_THROW(
            "Ray tracing validation is requested, but ray tracing is not "
            "supported");

#if FALCOR_HAS_D3D12
    if (mDesc.type == Type::D3D12) {
        // Get D3D device and attempt to enable ray tracing
        ID3D12Device5* pD3D12Device5 =
            static_cast<ID3D12Device5*>(getNativeHandle().as<ID3D12Device*>());
        auto res = NvAPI_D3D12_EnableRaytracingValidation(
            pD3D12Device5, NVAPI_D3D12_RAYTRACING_VALIDATION_FLAG_NONE);
        if (res == NVAPI_NOT_PERMITTED)
            FALCOR_THROW(
                "Failed to enable raytracing validation. Error code: {}.\nThis "
                "is typically caused when the "
                "NV_ALLOW_RAYTRACING_VALIDATION=1 environment variable is not "
                "set",
                (int)res);
        else if (res != NVAPI_OK)
            FALCOR_THROW(
                "Failed to enable raytracing validation. Error code: {}",
                (int)res);

        // Attempt to register the debug callback
        res = NvAPI_D3D12_RegisterRaytracingValidationMessageCallback(
            pD3D12Device5,
            &RaytracingValidationCallback,
            nullptr,
            &mpRayTraceValidationHandle);
        if (res != NVAPI_OK)
            FALCOR_THROW(
                "Failed to register raytracing validation callback. Error "
                "code: {}",
                (int)res);
    }
#endif

#if FALCOR_HAS_VULKAN
    if (mDesc.type == Type::Vulkan) {
        // TODO
    }
#endif

#endif
}

void Device::disableRaytracingValidation()
{
#if FALCOR_NVAPI_AVAILABLE && FALCOR_HAS_D3D12
    if (mpRayTraceValidationHandle) {
        ID3D12Device5* pD3D12Device5 =
            static_cast<ID3D12Device5*>(getNativeHandle().as<ID3D12Device*>());
        NvAPI_D3D12_UnregisterRaytracingValidationMessageCallback(
            pD3D12Device5, mpRayTraceValidationHandle);
        mpRayTraceValidationHandle = nullptr;
    }
#endif
}

void Device::flushRaytracingValidation()
{
#if FALCOR_NVAPI_AVAILABLE && FALCOR_HAS_D3D12
    if (mpRayTraceValidationHandle) {
        ID3D12Device5* pD3D12Device5 =
            static_cast<ID3D12Device5*>(getNativeHandle().as<ID3D12Device*>());
        NvAPI_D3D12_FlushRaytracingValidationMessages(pD3D12Device5);
    }
#endif
}

}  // namespace Falcor
