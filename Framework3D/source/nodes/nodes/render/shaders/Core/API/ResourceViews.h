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
#include <vector>

#include "Core/Macros.h"
#include "Core/Object.h"
#include "Core/Program/ProgramReflection.h"
#include "slang-com-ptr.h"

namespace Falcor {
struct FALCOR_API ResourceViewInfo {
    ResourceViewInfo() = default;
    ResourceViewInfo(
        uint32_t mostDetailedMip,
        uint32_t mipCount,
        uint32_t firstArraySlice,
        uint32_t arraySize)
        : mostDetailedMip(mostDetailedMip),
          mipCount(mipCount),
          firstArraySlice(firstArraySlice),
          arraySize(arraySize)
    {
    }

    ResourceViewInfo(uint64_t offset, uint64_t size)
        : offset(offset),
          size(size)
    {
    }

    static constexpr uint32_t kMaxPossible = -1;
    static constexpr uint64_t kEntireBuffer = -1;

    // Textures
    uint32_t mostDetailedMip = 0;
    uint32_t mipCount = kMaxPossible;
    uint32_t firstArraySlice = 0;
    uint32_t arraySize = kMaxPossible;

    // Buffers
    uint64_t offset = 0;
    uint64_t size = kEntireBuffer;

    bool operator==(const ResourceViewInfo& other) const
    {
        return (firstArraySlice == other.firstArraySlice) &&
               (arraySize == other.arraySize) && (mipCount == other.mipCount) &&
               (mostDetailedMip == other.mostDetailedMip) &&
               (offset == other.offset) && (size == other.size);
    }
};

/**
 * Abstracts API resource views.
 */
class FALCOR_API ResourceView : public Object {
    FALCOR_OBJECT(ResourceView)
   public:
    using Dimension = ReflectionResourceType::Dimensions;
    static const uint32_t kMaxPossible = -1;
    static constexpr uint64_t kEntireBuffer = -1;
    virtual ~ResourceView();

    ResourceView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView,
        uint32_t mostDetailedMip,
        uint32_t mipCount,
        uint32_t firstArraySlice,
        uint32_t arraySize)
        : mpDevice(pDevice),
          mGfxResourceView(gfxResourceView),
          mViewInfo(mostDetailedMip, mipCount, firstArraySlice, arraySize),
          mpResource(pResource)
    {
    }

    ResourceView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView,
        uint64_t offset,
        uint64_t size)
        : mpDevice(pDevice),
          mGfxResourceView(gfxResourceView),
          mViewInfo(offset, size),
          mpResource(pResource)
    {
    }

    ResourceView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView)
        : mpDevice(pDevice),
          mGfxResourceView(gfxResourceView),
          mpResource(pResource)
    {
    }

    nvrhi::BindingSetItem getGfxResourceView() const
    {
        return mGfxResourceView;
    }

    /**
     * Get information about the view.
     */
    const ResourceViewInfo& getViewInfo() const
    {
        return mViewInfo;
    }

    /**
     * Get the resource referenced by the view.
     */
    IResource* getResource() const
    {
        return mpResource;
    }

   protected:
    friend class Resource;

    void invalidate();

    IDevice* mpDevice;
    nvrhi::BindingSetItem mGfxResourceView;
    ResourceViewInfo mViewInfo;
    IResource* mpResource;
};

class FALCOR_API ShaderResourceView : public ResourceView {
   public:
    static nvrhi::BindingSetItem create(
        IDevice* pDevice,
        Texture* pTexture,
        uint32_t mostDetailedMip,
        uint32_t mipCount,
        uint32_t firstArraySlice,
        uint32_t arraySize);
    static nvrhi::BindingSetItem
    create(IDevice* pDevice, Buffer* pBuffer, uint64_t offset, uint64_t size);
    static nvrhi::BindingSetItem create(
        IDevice* pDevice,
        Dimension dimension);

   private:
    ShaderResourceView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView,
        uint32_t mostDetailedMip,
        uint32_t mipCount,
        uint32_t firstArraySlice,
        uint32_t arraySize)
        : ResourceView(
              pDevice,
              pResource,
              gfxResourceView,
              mostDetailedMip,
              mipCount,
              firstArraySlice,
              arraySize)
    {
    }
    ShaderResourceView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView,
        uint64_t offset,
        uint64_t size)
        : ResourceView(pDevice, pResource, gfxResourceView, offset, size)
    {
    }
    ShaderResourceView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView)
        : ResourceView(pDevice, pResource, gfxResourceView)
    {
    }
};

class FALCOR_API DepthStencilView : public ResourceView {
   public:
    static ref<DepthStencilView> create(
        IDevice* pDevice,
        Texture* pTexture,
        uint32_t mipLevel,
        uint32_t firstArraySlice,
        uint32_t arraySize);
    static ref<DepthStencilView> create(IDevice* pDevice, Dimension dimension);

   private:
    DepthStencilView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView,
        uint32_t mipLevel,
        uint32_t firstArraySlice,
        uint32_t arraySize)
        : ResourceView(
              pDevice,
              pResource,
              gfxResourceView,
              mipLevel,
              1,
              firstArraySlice,
              arraySize)
    {
    }
};

class FALCOR_API UnorderedAccessView : public ResourceView {
   public:
    static nvrhi::BindingSetItem create(
        IDevice* pDevice,
        Texture* pTexture,
        uint32_t mipLevel,
        uint32_t firstArraySlice,
        uint32_t arraySize);
    static nvrhi::BindingSetItem
    create(IDevice* pDevice, Buffer* pBuffer, uint64_t offset, uint64_t size);
    static nvrhi::BindingSetItem create(
        IDevice* pDevice,
        Dimension dimension);

   private:
    UnorderedAccessView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView,
        uint32_t mipLevel,
        uint32_t firstArraySlice,
        uint32_t arraySize)
        : ResourceView(
              pDevice,
              pResource,
              gfxResourceView,
              mipLevel,
              1,
              firstArraySlice,
              arraySize)
    {
    }

    UnorderedAccessView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView,
        uint64_t offset,
        uint64_t size)
        : ResourceView(pDevice, pResource, gfxResourceView, offset, size)
    {
    }
};

class FALCOR_API RenderTargetView : public ResourceView {
   public:
    static ref<RenderTargetView> create(
        IDevice* pDevice,
        Texture* pTexture,
        uint32_t mipLevel,
        uint32_t firstArraySlice,
        uint32_t arraySize);
    static ref<RenderTargetView> create(IDevice* pDevice, Dimension dimension);

   private:
    RenderTargetView(
        IDevice* pDevice,
        IResource* pResource,
        nvrhi::BindingSetItem gfxResourceView,
        uint32_t mipLevel,
        uint32_t firstArraySlice,
        uint32_t arraySize)
        : ResourceView(
              pDevice,
              pResource,
              gfxResourceView,
              mipLevel,
              1,
              firstArraySlice,
              arraySize)
    {
    }
};
}  // namespace Falcor
