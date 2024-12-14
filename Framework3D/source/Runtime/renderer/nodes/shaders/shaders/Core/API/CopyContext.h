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
#include <memory>
#include <string>
#include <vector>

#include "Core/Macros.h"
#include "Device.h"
#include "utils/Math/VectorTypes.h"
#include "utils/CudaRuntime.h"

#if FALCOR_HAS_CUDA
struct CUstream_st;
typedef CUstream_st* cudaStream_t;
#endif

namespace USTC_CG {
class Profiler;

class HD_USTC_CG_API CopyContext {
   public:
    class HD_USTC_CG_API ReadTextureTask {
       public:
        using SharedPtr = std::shared_ptr<ReadTextureTask>;
        static SharedPtr
        create(CopyContext* pCtx, Texture* pTexture, uint32_t subresourceIndex);
        void getData(void* pData, size_t size) const;
        std::vector<uint8_t> getData() const;

       private:
        ReadTextureTask() = default;
        nvrhi::BufferHandle mpBuffer;
        CopyContext* mpContext;
        uint32_t mRowCount;
        uint32_t mRowSize;
        uint32_t mActualRowSize;
        uint32_t mDepth;
    };

    /**
     * Constructor.
     * Throws an exception if creation failed.
     * @param[in] pDevice Graphics device.
     * @param[in] pQueue Command queue.
     */
    CopyContext(Device* pDevice, nvrhi::CommandListHandle pQueue);
    virtual ~CopyContext();

    ref<Device> getDevice() const;

    /**
     * Flush the command list. This doesn't reset the command allocator, just
     * submits the commands
     * @param[in] wait If true, will block execution until the GPU finished
     * processing the commands
     */
    virtual void submit(bool wait = false);

    /**
     * Wait for the CUDA stream to finish execution.
     * Queues a device-side wait on the command queue and adds an async fence
     * signal on the CUDA stream. Returns immediately.
     * @param stream The CUDA stream to wait for.
     */
    void waitForCuda(cudaStream_t stream = 0);

    /**
     * Wait for the Falcor command queue to finish execution.
     * Queues a device-side signal on the command queue and adds an async fence
     * wait on the CUDA stream. Returns immediately.
     * @param stream The CUDA stream to wait on.
     */
    void waitForFalcor(cudaStream_t stream = 0);

    /**
     * Insert a resource barrier
     * if pViewInfo is nullptr, will transition the entire resource. Otherwise,
     * it will only transition the subresource in the view
     * @return true if a barrier commands were recorded for the entire
     * resource-view, otherwise false (for example, when the current resource
     * state is the same as the new state or when only some subresources were
     * transitioned)
     */
    virtual void resourceBarrier(
        Resource* pResource,
        ResourceStates newState,
        const void* pViewInfo = nullptr);

    virtual void resourceBarrier(
        Texture* pTexture,
        ResourceStates newState,
        const nvrhi::TextureSubresourceSet* pViewInfo = nullptr);

    virtual void resourceBarrier(
        Buffer* pResource,
        ResourceStates newState,
        const nvrhi::BufferRange* pViewInfo = nullptr);

    /**
     * Insert a UAV barrier
     */
    virtual void uavBarrier(Resource* pResource);

    /**
     * Copy an entire resource
     */
    void copyResource(Resource* pDst, Resource* pSrc);

    /**
     * Copy a subresource
     */
    void copySubresource(
        Texture* pDst,
        uint32_t dstSubresourceIdx,
        Texture* pSrc,
        uint32_t srcSubresourceIdx);

    /**
     * Copy part of a buffer
     */
    void copyBufferRegion(
        Buffer* pDst,
        uint64_t dstOffset,
        Buffer* pSrc,
        uint64_t srcOffset,
        uint64_t numBytes);

    /**
     * Copy a region of a subresource from one texture to another
     * `srcOffset`, `dstOffset` and `size` describe the source and destination
     * regions. For any channel of `extent` that is -1, the source texture
     * dimension will be used
     */
    void copySubresourceRegion(
        Texture* pDst,
        uint32_t dstSubresource,
        Texture* pSrc,
        uint32_t srcSubresource,
        const uint3& dstOffset = uint3(0),
        const uint3& srcOffset = uint3(0),
        const uint3& size = uint3(-1));

    /**
     * Update an entire texture
     */
    void updateTextureData(Texture* pTexture, const void* pData);

    /**
     * Update a buffer
     */
    void updateBuffer(
        Buffer* pBuffer,
        const void* pData,
        size_t offset = 0,
        size_t numBytes = 0);

    void readBuffer(
        Buffer* pBuffer,
        void* pData,
        size_t offset = 0,
        size_t numBytes = 0);

    template<typename T>
    std::vector<T> readBuffer(
        Buffer* pBuffer,
        size_t firstElement = 0,
        size_t elementCount = 0)
    {
        if (elementCount == 0)
            elementCount = pBuffer->getDesc().byteSize / sizeof(T);

        size_t offset = firstElement * sizeof(T);
        size_t numBytes = elementCount * sizeof(T);

        std::vector<T> result(elementCount);
        readBuffer(pBuffer, result.data(), offset, numBytes);
        return result;
    }

    /**
     * Read texture data synchronously. Calling this command will flush the
     * pipeline and wait for the GPU to finish execution
     */
    std::vector<uint8_t> readTextureSubresource(
        Texture* pTexture,
        uint32_t subresourceIndex);

    /**
     * Read texture data Asynchronously
     */
    ReadTextureTask::SharedPtr asyncReadTextureSubresource(
        Texture* pTexture,
        uint32_t subresourceIndex);

    /**
     * Add an aftermath marker to the command list.
     */
    void addAftermathMarker(std::string_view name);

    nvrhi::ICommandList* getCommandList() const
    {
        return mpLowLevelData.Get();
    }

   protected:
    void updateTextureSubresources(
        Texture* pTexture,
        uint32_t firstSubresource,
        uint32_t subresourceCount,
        const void* pData,
        const uint3& offset = uint3(0),
        const uint3& size = uint3(-1));

    void subresourceBarriers(
        Texture* pTexture,
        ResourceStates newState,
        const nvrhi::TextureSubresourceSet* pViewInfo);
    void apiSubresourceBarrier(
        Texture* pTexture,
        ResourceStates newState,
        ResourceStates oldState,
        uint32_t arraySlice,
        uint32_t mipLevel);

    Device* mpDevice;
    nvrhi::CommandListHandle mpLowLevelData;
};
}  // namespace USTC_CG
