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
#pragma once
#include "Core/Macros.h"
#include "Core/Object.h"

#if FALCOR_HAS_D3D12
#include <guiddef.h>
#endif

#include <array>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include "Core/Enum.h"
#include "Core/Platform/OS.h"
#include "Utils/Math/VectorTypes.h"
#include "slang-com-ptr.h"

namespace Falcor {
class RtStateObject;
class GraphicsStateObject;
struct GraphicsStateObjectDesc;
struct ComputeStateObjectDesc;
class ComputeStateObject;
struct RtStateObjectDesc;
#ifdef _DEBUG
#define FALCOR_DEFAULT_ENABLE_DEBUG_LAYER true
#else
#define FALCOR_DEFAULT_ENABLE_DEBUG_LAYER false
#endif

#if FALCOR_HAS_D3D12
class D3D12DescriptorPool;
#endif

class PipelineCreationAPIDispatcher;
class ProgramManager;
class Profiler;
class AftermathContext;
class RenderContext;
namespace cuda_utils {
    class CudaDevice;
};

/// Holds the adapter LUID (or UUID).
/// Note: The adapter LUID is actually just 8 bytes, but on Linux the LUID is
/// not supported, so we use this to store the 16-byte UUID instead.
struct AdapterLUID {
    std::array<uint8_t, 16> luid;

    AdapterLUID()
    {
        luid.fill(0);
    }
    bool isValid() const
    {
        return *this != AdapterLUID();
    }
    bool operator==(const AdapterLUID& other) const
    {
        return luid == other.luid;
    }
    bool operator!=(const AdapterLUID& other) const
    {
        return luid != other.luid;
    }
    bool operator<(const AdapterLUID& other) const
    {
        return luid < other.luid;
    }
};

struct AdapterInfo {
    /// Descriptive name of the adapter.
    std::string name;

    /// Unique identifier for the vendor.
    uint32_t vendorID;

    // Unique identifier for the physical device among devices from the vendor.
    uint32_t deviceID;

    // Logically unique identifier of the adapter.
    AdapterLUID luid;
};

class FALCOR_API Device : public Object {
    FALCOR_OBJECT(Device)

    auto getGraphicsAPI()
    {
        return mGfxDevice->getGraphicsAPI();
    }

   public:
    /**
     * Maximum number of in-flight frames.
     * Typically there are at least two frames, one being rendered to, the other
     * being presented. We add one more to be on the save side.
     */
    static constexpr uint32_t kInFlightFrameCount = 3;

    /// Device type.
    enum Type {
        Default,  ///< Default device type, favors D3D12 over Vulkan.
        D3D12,
        Vulkan,
    };
    FALCOR_ENUM_INFO(
        Type,
        {
            { Type::Default, "Default" },
            { Type::D3D12, "D3D12" },
            { Type::Vulkan, "Vulkan" },
        });

    /// Device descriptor.
    struct Desc {
        /// The device type (D3D12/Vulkan).
        Type type = Type::Default;

        /// GPU index (indexing into GPU list returned by getGPUList()).
        uint32_t gpu = 0;

        /// Enable the debug layer. The default for release build is false, for
        /// debug build it's true.
        bool enableDebugLayer = FALCOR_DEFAULT_ENABLE_DEBUG_LAYER;

        /// Enable NVIDIA NSight Aftermath GPU crash dump.
        bool enableAftermath = false;

        /// The maximum number of entries allowable in the shader cache. A value
        /// of 0 indicates no limit.
        uint32_t maxShaderCacheEntryCount = 1000;

        /// The full path to the root directory for the shader cache. An empty
        /// string will disable the cache.
        std::string shaderCachePath =
            (getRuntimeDirectory() / ".shadercache").string();

#if FALCOR_HAS_D3D12
        /// GUID list for experimental features
        std::vector<GUID> experimentalFeatures;
#endif

        /// Whether to enable ray tracing validation (requires NVAPI)
        bool enableRaytracingValidation = false;
    };

    struct Info {
        std::string adapterName;
        AdapterLUID adapterLUID;
        std::string apiName;
    };

    struct Limits {
        uint3 maxComputeDispatchThreadGroups;
        uint32_t maxShaderVisibleSamplers;
    };

    /**
     * Constructor. Throws an exception if creation failed.
     * @param[in] desc Device configuration descriptor.
     */
    Device(const Desc& desc);
    ~Device();

    /// Create a compute state object.
    ref<ComputeStateObject> createComputeStateObject(
        const ComputeStateObjectDesc& desc);
    /// Create a graphics state object.
    ref<GraphicsStateObject> createGraphicsStateObject(
        const GraphicsStateObjectDesc& desc);
    /// Create a raytracing state object.
    ref<RtStateObject> createRtStateObject(const RtStateObjectDesc& desc);

    ProgramManager* getProgramManager() const
    {
        return mpProgramManager.get();
    }

    Profiler* getProfiler() const
    {
        return mpProfiler.get();
    }

    /**
     * Get the default render-context.
     * The default render-context is managed completely by the device. The user
     * should just queue commands into it, the device will take care of
     * allocation, submission and synchronization
     */
    RenderContext* getRenderContext() const
    {
        return mpRenderContext.get();
    }

    /// Returns the global slang session.
    slang::IGlobalSession* getSlangGlobalSession() const
    {
        return mSlangGlobalSession;
    }

    /// Return the GFX define.
    nvrhi::IDevice* getGfxDevice() const
    {
        return mGfxDevice;
    }

    /// Return the GFX command queue.
    nvrhi::ICommandList* getGfxCommandQueue() const
    {
        return mGfxCommandQueue.Get();
    }

    /**
     * End a frame.
     * This closes the current command buffer, switches to a new heap for
     * transient resources and opens a new command buffer. This also executes
     * deferred releases of resources from past frames.
     */
    void endFrame();

    /**
     * Flushes pipeline, releases resources, and blocks until completion
     */
    void wait();

    /**
     * Get the desc
     */
    const Desc& getDesc() const
    {
        return mDesc;
    }

    /**
     * Get the device type.
     */
    Type getType() const
    {
        return mDesc.type;
    }

    /**
     * Throws an exception if the device is not a D3D12 device.
     */
    void requireD3D12() const;

    /**
     * Throws an exception if the device is not a Vulkan device.
     */
    void requireVulkan() const;

    /**
     * Get an object that represents a default sampler.
     */
    const nvrhi::SamplerHandle& getDefaultSampler() const
    {
        return mpDefaultSampler;
    }

#if FALCOR_HAS_AFTERMATH
    AftermathContext* getAftermathContext() const
    {
        return mpAftermathContext.get();
    }
#endif

#if FALCOR_HAS_D3D12
    const ref<D3D12DescriptorPool>& getD3D12CpuDescriptorPool() const
    {
        requireD3D12();
        return mpD3D12CpuDescPool;
    }
    const ref<D3D12DescriptorPool>& getD3D12GpuDescriptorPool() const
    {
        requireD3D12();
        return mpD3D12GpuDescPool;
    }
#endif  // FALCOR_HAS_D3D12

    const ref<GpuMemoryHeap>& getUploadHeap() const
    {
        return mpUploadHeap;
    }
    const ref<GpuMemoryHeap>& getReadBackHeap() const
    {
        return mpReadBackHeap;
    }
    const ref<QueryHeap>& getTimestampQueryHeap() const
    {
        return mpTimestampQueryHeap;
    }
    void releaseResource(ISlangUnknown* pResource);

    double getGpuTimestampFrequency() const
    {
        return mGpuTimestampFrequency;
    }  // ms/tick

    const Info& getInfo() const
    {
        return mInfo;
    }

    /**
     * Get the device limits.
     */
    const Limits& getLimits() const
    {
        return mLimits;
    }

    /// Get the texture row memory alignment in bytes.
    size_t getTextureRowAlignment() const;

#if FALCOR_HAS_CUDA
    /// Initialize CUDA device sharing the same adapter as the graphics device.
    bool initCudaDevice();

    /// Get the CUDA device sharing the same adapter as the graphics device.
    cuda_utils::CudaDevice* getCudaDevice() const;
#endif

    /// Report live objects in GFX.
    /// This is useful for checking clean shutdown where all resources are
    /// properly released.
    static void reportLiveObjects();

    /**
     * Try to enable D3D12 Agility SDK at runtime.
     * Note: This must be called before creating a device to have any effect.
     *
     * Prefer adding FALCOR_EXPORT_D3D12_AGILITY_SDK to the main translation
     * unit of executables to tag the application binary to load the D3D12
     * Agility SDK.
     *
     * If loading Falcor as a library only (the case when loading Falcor as a
     * Python module) tagging the main application (Python interpreter) is not
     * possible. The alternative is to use the D3D12SDKConfiguration API
     * introduced in Windows SDK 20348. This however requires "Developer Mode"
     * to be enabled and the executed Python interpreter to be stored on the
     * same drive as Falcor.
     *
     * @return Return true if D3D12 Agility SDK was successfully enabled.
     */
    static bool enableAgilitySDK();

    /**
     * Get a list of all available GPUs.
     */
    static std::vector<AdapterInfo> getGPUs(Type deviceType);

    /**
     * Get the global device mutex.
     * WARNING: Falcor is generally not thread-safe. This mutex is used in very
     * specific places only, currently only for doing parallel texture loading.
     */
    std::mutex& getGlobalGfxMutex()
    {
        return mGlobalGfxMutex;
    }

    /**
     * When ray tracing validation is enabled, call this to force validation
     * messages to be flushed. It is automatically called at the end of each
     * frame. Only messages from completed work will flush, so to guaruntee all
     * are printed, it must be called after a fence. NOTE: This has no effect on
     * Vulkan, in which the driver flushes automatically at device idle/lost.
     */
    void flushRaytracingValidation();

   private:
    struct ResourceRelease {
        uint64_t fenceValue;
        Slang::ComPtr<ISlangUnknown> mObject;
    };
    std::queue<ResourceRelease> mDeferredReleases;

    void executeDeferredReleases();

    void enableRaytracingValidation();
    void disableRaytracingValidation();

    Desc mDesc;
    Slang::ComPtr<slang::IGlobalSession> mSlangGlobalSession;
    Slang::ComPtr<nvrhi::IDevice> mGfxDevice;
    nvrhi::CommandListHandle mGfxCommandQueue;

    uint32_t mCurrentTransientResourceHeapIndex = 0;

    nvrhi::SamplerHandle mpDefaultSampler;
    ref<GpuMemoryHeap> mpUploadHeap;
    ref<GpuMemoryHeap> mpReadBackHeap;
    ref<QueryHeap> mpTimestampQueryHeap;
#if FALCOR_HAS_D3D12
    ref<D3D12DescriptorPool> mpD3D12CpuDescPool;
    ref<D3D12DescriptorPool> mpD3D12GpuDescPool;
#endif
    nvrhi::EventQueryHandle mpFrameFence;

    std::unique_ptr<RenderContext> mpRenderContext;
    double mGpuTimestampFrequency;

    Info mInfo;
    Limits mLimits;

#if FALCOR_HAS_AFTERMATH
    std::unique_ptr<AftermathContext> mpAftermathContext;
#endif

#if FALCOR_NVAPI_AVAILABLE
    std::unique_ptr<PipelineCreationAPIDispatcher> mpAPIDispatcher;
#endif

    std::unique_ptr<ProgramManager> mpProgramManager;
    std::unique_ptr<Profiler> mpProfiler;

#if FALCOR_NVAPI_AVAILABLE && FALCOR_HAS_D3D12
    void* mpRayTraceValidationHandle = nullptr;
#endif

#if FALCOR_HAS_CUDA
    /// CUDA device sharing the same adapter as the graphics device.
    mutable ref<cuda_utils::CudaDevice> mpCudaDevice;
#endif

    std::mutex mGlobalGfxMutex;
};

inline constexpr uint32_t getMaxViewportCount()
{
    return 8;
}

FALCOR_ENUM_REGISTER(Device::Type);

}  // namespace Falcor
