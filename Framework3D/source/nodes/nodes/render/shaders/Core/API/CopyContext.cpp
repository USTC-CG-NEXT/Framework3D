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
#include "CopyContext.h"

#include "Core/Error.h"
#include "Device.h"
#include "Utils/Logger.h"
#include "Utils/Logging/Logging.h"
#include "Utils/Math/Common.h"
#include "Utils/Math/VectorMath.h"
#include "nvrhi/common/misc.h"
#include "nvrhi/utils.h"
#include "utils/CudaUtils.h"

#if FALCOR_HAS_CUDA
#include "Utils/CudaUtils.h"
#endif

namespace Falcor {
CopyContext::CopyContext(Device* pDevice, nvrhi::CommandListHandle pQueue)
    : mpDevice(pDevice),
      mpLowLevelData(pQueue)
{
    FALCOR_ASSERT(mpDevice);
    FALCOR_ASSERT(pQueue);
}

CopyContext::~CopyContext() = default;

ref<Device> CopyContext::getDevice() const
{
    return ref<Device>(mpDevice);
}

void CopyContext::submit(bool wait)
{
    mpLowLevelData->close();
    getDevice()->getNvrhiDevice()->executeCommandList(mpLowLevelData);

    if (wait) {
        getDevice()->getNvrhiDevice()->waitForIdle();
    }
}

void CopyContext::waitForCuda(cudaStream_t stream)
{
    if (mpDevice->getNvrhiDevice()->getGraphicsAPI() ==
        nvrhi::GraphicsAPI::D3D12) {
        USTC_CG::logging("waitForCuda not implemented");
        // mpLowLevelData->getCudaSemaphore()->waitForCuda(this, stream);
    }
    else {
        // In the past, we used to wait for all CUDA work to be done.
        // Since GFX with Vulkan doesn't support shared fences yet, we do the
        // same here.
        cuda_utils::deviceSynchronize();
    }
}

void CopyContext::waitForFalcor(cudaStream_t stream)
{
    if (mpDevice->getGraphicsAPI() == nvrhi::GraphicsAPI::D3D12) {
        getDevice()->executeCommandList(mpLowLevelData);
        USTC_CG::logging("CUDA Semephore not implemented;");
        // mpLowLevelData->getCudaSemaphore()->waitForFalcor(this, stream);
    }
    else {
        // In the past, we used to wait for all work on the command queue to be
        // done. Since GFX with Vulkan doesn't support shared fences yet, we do
        // the same here.
        submit(true);
    }
}

void CopyContext::resourceBarrier(
    Resource* pResource,
    ResourceStates newState,
    const void* pViewInfo)
{
    auto buffer = dynamic_cast<nvrhi::IBuffer*>(pResource);
    if (buffer) {
        resourceBarrier(
            buffer,
            newState,
            static_cast<const nvrhi::BufferRange*>(pViewInfo));
    }
    else {
        auto texture = nvrhi::checked_cast<nvrhi::ITexture*>(pResource);
        resourceBarrier(
            texture,
            newState,
            static_cast<const nvrhi::TextureSubresourceSet*>(pViewInfo));
    }
}

CopyContext::ReadTextureTask::SharedPtr
CopyContext::asyncReadTextureSubresource(
    Texture* pTexture,
    uint32_t subresourceIndex)
{
    return CopyContext::ReadTextureTask::create(
        this, pTexture, subresourceIndex);
}

std::vector<uint8_t> CopyContext::readTextureSubresource(
    Texture* pTexture,
    uint32_t subresourceIndex)
{
    CopyContext::ReadTextureTask::SharedPtr pTask =
        asyncReadTextureSubresource(pTexture, subresourceIndex);
    return pTask->getData();
}

void CopyContext::resourceBarrier(
    Texture* pTexture,
    ResourceStates newState,
    const nvrhi::TextureSubresourceSet* pViewInfo)
{
    if (pViewInfo) {
        getCommandList()->setTextureState(pTexture, *pViewInfo, newState);
    }
    else {
        getCommandList()->setTextureState(pTexture, {}, newState);
    }
}

void CopyContext::resourceBarrier(
    Buffer* pResource,
    ResourceStates newState,
    const nvrhi::BufferRange* pViewInfo)
{
    auto pBuffer = nvrhi::checked_cast<nvrhi::IBuffer*>(pResource);
    getCommandList()->setBufferState(pBuffer, newState);
}

void CopyContext::subresourceBarriers(
    Texture* pTexture,
    ResourceStates newState,
    const nvrhi::TextureSubresourceSet* pViewInfo)
{
    getCommandList()->setTextureState(pTexture, *pViewInfo, newState);
}

void CopyContext::updateTextureData(Texture* pTexture, const void* pData)
{
    uint32_t subresourceCount =
        pTexture->getDesc().arraySize * pTexture->getDesc().mipLevels;
    if (pTexture->getDesc().dimension == nvrhi::TextureDimension::TextureCube) {
        subresourceCount *= 6;
    }

    updateTextureSubresources(pTexture, 0, subresourceCount, pData);
}

CopyContext::ReadTextureTask::SharedPtr CopyContext::ReadTextureTask::create(
    CopyContext* pCtx,
    Texture* pTexture,
    uint32_t subresourceIndex)
{
    nvrhi::utils::NotImplemented();

    // SharedPtr pThis = SharedPtr(new ReadTextureTask);
    // pThis->mpContext = pCtx;
    //// Get footprint
    // auto srcTexture = pTexture;
    // nvrhi::FormatInfo formatInfo =
    //     nvrhi::getFormatInfo(srcTexture->getDesc().format);

    // auto mipLevel = pTexture->getSubresourceMipLevel(subresourceIndex);
    // pThis->mActualRowSize = uint32_t(
    //     (pTexture->getWidth(mipLevel) + formatInfo.blockWidth - 1) /
    //     formatInfo.blockWidth * formatInfo.blockSizeInBytes);
    // size_t rowAlignment = 1;
    // pCtx->mpDevice->getNvrhiDevice()->getTextureRowAlignment(&rowAlignment);
    // pThis->mRowSize =
    //     align_to(static_cast<uint32_t>(rowAlignment), pThis->mActualRowSize);
    // uint64_t rowCount =
    //     (pTexture->getHeight(mipLevel) + formatInfo.blockHeight - 1) /
    //     formatInfo.blockHeight;
    // uint64_t size = pTexture->getDepth(mipLevel) * rowCount *
    // pThis->mRowSize;

    //// Create buffer
    // pThis->mpBuffer = pCtx->getDevice()->createBuffer(
    //     size, ResourceBindFlags::None, MemoryType::ReadBack, nullptr);

    //// Copy from texture to buffer
    // pCtx->resourceBarrier(pTexture, ResourceStates::CopySource, TODO);
    // auto encoder = pCtx->mpLowLevelData;
    // nvrhi::TextureSubresourceSet srcSubresource = {};
    // srcSubresource.baseArraySlice =
    //     pTexture->getSubresourceArraySlice(subresourceIndex);
    // srcSubresource.baseMipLevel = mipLevel;
    // srcSubresource.numArraySlices = 1;
    // srcSubresource.numMipLevels = 1;
    // encoder->copyTextureToBuffer(
    //     pThis->mpBuffer,
    //     0,
    //     size,
    //     pThis->mRowSize,
    //     srcTexture,
    //     ResourceStates::CopySource,
    //     srcSubresource,
    //     nvrhi::TextureSlice(0, 0, 0),
    //     Extents{ static_cast<unsigned>(pTexture->getWidth(mipLevel)),
    //              static_cast<unsigned>(pTexture->getHeight(mipLevel)),
    //              static_cast<unsigned>(pTexture->getDepth(mipLevel)) });
    // pCtx->setPendingCommands(true);

    //// Create a fence and signal
    // pThis->mpFence = pCtx->getDevice()->createFence();
    // pThis->mpFence->breakStrongReferenceToDevice();
    // pCtx->submit(false);
    // pCtx->signal(pThis->mpFence.get());
    // pThis->mRowCount = (uint32_t)rowCount;
    // pThis->mDepth = pTexture->getDepth(mipLevel);
    // return pThis;
}

void CopyContext::ReadTextureTask::getData(void* pData, size_t size) const
{
    FALCOR_ASSERT(size == size_t(mRowCount) * mActualRowSize * mDepth);

    uint8_t* pDst = reinterpret_cast<uint8_t*>(pData);
    const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(
        mpContext->getDevice()->getNvrhiDevice()->mapBuffer(
            mpBuffer, nvrhi::CpuAccessMode::Read));

    for (uint32_t z = 0; z < mDepth; z++) {
        const uint8_t* pSrcZ = pSrc + z * (size_t)mRowSize * mRowCount;
        uint8_t* pDstZ = pDst + z * (size_t)mActualRowSize * mRowCount;
        for (uint32_t y = 0; y < mRowCount; y++) {
            const uint8_t* pSrcY = pSrcZ + y * (size_t)mRowSize;
            uint8_t* pDstY = pDstZ + y * (size_t)mActualRowSize;
            std::memcpy(pDstY, pSrcY, mActualRowSize);
        }
    }
    mpContext->getDevice()->getNvrhiDevice()->unmapBuffer(mpBuffer);
}

std::vector<uint8_t> CopyContext::ReadTextureTask::getData() const
{
    std::vector<uint8_t> result(size_t(mRowCount) * mActualRowSize * mDepth);
    getData(result.data(), result.size());
    return result;
}

void CopyContext::updateTextureSubresources(
    Texture* pTexture,
    uint32_t firstSubresource,
    uint32_t subresourceCount,
    const void* pData,
    const uint3& offset,
    const uint3& size)
{
    nvrhi::utils::NotImplemented();
}

void CopyContext::apiSubresourceBarrier(
    Texture* pTexture,
    ResourceStates newState,
    ResourceStates oldState,
    uint32_t arraySlice,
    uint32_t mipLevel)
{
    auto resourceEncoder = mpLowLevelData;
    auto subresourceState = getCommandList()->getTextureSubresourceState(
        pTexture, arraySlice, mipLevel);
    if (subresourceState != newState) {
        auto* textureResource = pTexture;
        nvrhi::TextureSubresourceSet textureSubresourceSet = {};
        textureSubresourceSet.baseArraySlice = arraySlice;
        textureSubresourceSet.baseMipLevel = mipLevel;
        textureSubresourceSet.numArraySlices = 1;
        textureSubresourceSet.numMipLevels = 1;
        resourceEncoder->setTextureState(
            textureResource, textureSubresourceSet, newState);
    }
}

void CopyContext::uavBarrier(Resource* pResource)
{
    auto buffer = dynamic_cast<nvrhi::IBuffer*>(pResource);

    if (buffer) {
        nvrhi::utils::BufferUavBarrier(getCommandList(), buffer);
    }
    else {
        auto texture = nvrhi::checked_cast<nvrhi::ITexture*>(pResource);
        nvrhi::utils::TextureUavBarrier(getCommandList(), texture);
    }
}

void CopyContext::copyResource(Resource* pDst, Resource* pSrc)
{
    // Copy from texture to texture or from buffer to buffer.
    FALCOR_ASSERT(pDst->getType() == pSrc->getType());

    resourceBarrier(pDst, ResourceStates::CopyDest);
    resourceBarrier(pSrc, ResourceStates::CopySource);

    auto resourceEncoder = mpLowLevelData;

    auto buffer = dynamic_cast<const nvrhi::IBuffer*>(pDst);

    if (buffer) {
        Buffer* pSrcBuffer = nvrhi::checked_cast<Buffer*>(pSrc);
        Buffer* pDstBuffer = nvrhi::checked_cast<Buffer*>(pDst);

        FALCOR_ASSERT(pSrcBuffer->getSize() <= pDstBuffer->getSize());

        resourceEncoder->copyBuffer(
            pDstBuffer, 0, pSrcBuffer, 0, pSrcBuffer->getDesc().byteSize);
    }
    else {
        Texture* pSrcTexture = static_cast<Texture*>(pSrc);
        Texture* pDstTexture = static_cast<Texture*>(pDst);
        resourceEncoder->copyTexture(pDstTexture, {}, pSrcTexture, {});
    }
}

void CopyContext::copySubresource(
    Texture* pDst,
    uint32_t dstSubresourceIdx,
    Texture* pSrc,
    uint32_t srcSubresourceIdx)
{
    copySubresourceRegion(
        pDst,
        dstSubresourceIdx,
        pSrc,
        srcSubresourceIdx,
        uint3(0),
        uint3(0),
        uint3(-1));
}

void CopyContext::updateBuffer(
    Buffer* pBuffer,
    const void* pData,
    size_t offset,
    size_t numBytes)
{
    if (numBytes == 0) {
        numBytes = pBuffer->getDesc().byteSize - offset;
    }

    resourceBarrier(pBuffer, ResourceStates::CopyDest);
    auto resourceEncoder = mpLowLevelData;
    resourceEncoder->writeBuffer(pBuffer, (void*)pData, offset, numBytes);
}

void CopyContext::readBuffer(
    Buffer* pBuffer,
    void* pData,
    size_t offset,
    size_t numBytes)
{
    nvrhi::utils::NotImplemented();
    // if (numBytes == 0)
    //     numBytes = pBuffer->getSize() - offset;

    // if (pBuffer->adjustSizeOffsetParams(numBytes, offset) == false) {
    //     logWarning(
    //         "CopyContext::readBuffer() - size and offset are invalid. Nothing
    //         " "to read.");
    //     return;
    // }

    // const auto& pReadBackHeap = mpDevice->getReadBackHeap();

    // auto allocation = pReadBackHeap->allocate(numBytes);

    // resourceBarrier(pBuffer, ResourceStates::CopySource);

    // auto resourceEncoder = mpLowLevelData;
    // resourceEncoder->copyBuffer(
    //     allocation.gfxBufferResource,
    //     allocation.offset,
    //     pBuffer,
    //     offset,
    //     numBytes);

    // submit(true);

    // std::memcpy(pData, allocation.pData, numBytes);

    // pReadBackHeap->release(allocation);
}

void CopyContext::copyBufferRegion(
    Buffer* pDst,
    uint64_t dstOffset,
    Buffer* pSrc,
    uint64_t srcOffset,
    uint64_t numBytes)
{
    resourceBarrier(pDst, ResourceStates::CopyDest);
    resourceBarrier(pSrc, ResourceStates::CopySource);

    auto resourceEncoder = mpLowLevelData;
    resourceEncoder->copyBuffer(pDst, dstOffset, pSrc, srcOffset, numBytes);
}

void CopyContext::copySubresourceRegion(
    Texture* pDst,
    uint32_t dstSubresourceIdx,
    Texture* pSrc,
    uint32_t srcSubresourceIdx,
    const uint3& dstOffset,
    const uint3& srcOffset,
    const uint3& size)
{
    resourceBarrier(pDst, ResourceStates::CopyDest);
    resourceBarrier(pSrc, ResourceStates::CopySource);

    nvrhi::TextureSlice srcSlice;
    nvrhi::TextureSlice dstSlice;

    // Fill the slices with the provided offsets and sizes
    srcSlice.x = srcOffset.x;
    srcSlice.y = srcOffset.y;
    srcSlice.z = srcOffset.z;
    srcSlice.width = size.x;
    srcSlice.height = size.y;
    srcSlice.depth = size.z;
    srcSlice.setMipLevel(srcSubresourceIdx);  // Set the source mip level

    dstSlice.x = dstOffset.x;
    dstSlice.y = dstOffset.y;
    dstSlice.z = dstOffset.z;
    dstSlice.width = size.x;
    dstSlice.height = size.y;
    dstSlice.depth = size.z;
    dstSlice.setMipLevel(dstSubresourceIdx);  // Set the destination mip level

    USTC_CG::logging("copy requires refactor");

    getCommandList()->copyTexture(pDst, dstSlice, pSrc, srcSlice);
}

void CopyContext::addAftermathMarker(std::string_view name)
{
#if FALCOR_HAS_AFTERMATH
    if (AftermathContext* pAftermathContext = mpDevice->getAftermathContext())
        pAftermathContext->addMarker(mpLowLevelData.get(), name);
#endif
};

}  // namespace Falcor
