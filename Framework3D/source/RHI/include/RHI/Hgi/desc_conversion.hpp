#pragma once

#include <nvrhi/nvrhi.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgi/types.h>

#include "RHI/Hgi/format_conversion.hpp"
#include "RHI/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace RHI {
inline nvrhi::TextureDesc ConvertToNvrhiTextureDesc(
    const pxr::HgiTextureDesc& hgiDesc)
{
    nvrhi::TextureDesc nvrhiDesc;
    nvrhiDesc.width = hgiDesc.dimensions[0];
    nvrhiDesc.height = hgiDesc.dimensions[1];
    nvrhiDesc.depth = hgiDesc.dimensions[2];
    nvrhiDesc.arraySize = hgiDesc.layerCount;
    nvrhiDesc.mipLevels = hgiDesc.mipLevels;
    nvrhiDesc.sampleCount = hgiDesc.sampleCount;
    nvrhiDesc.format = ConvertToNvrhiFormat(hgiDesc.format);
    nvrhiDesc.dimension = (hgiDesc.type == pxr::HgiTextureType3D)
                              ? nvrhi::TextureDimension::Texture3D
                          : (hgiDesc.type == pxr::HgiTextureType2D)
                              ? nvrhi::TextureDimension::Texture2D
                              : nvrhi::TextureDimension::Texture1D;
    nvrhiDesc.debugName = hgiDesc.debugName;
    nvrhiDesc.isShaderResource =
        (hgiDesc.usage & pxr::HgiTextureUsageBitsShaderRead) != 0;
    nvrhiDesc.isRenderTarget =
        (hgiDesc.usage & pxr::HgiTextureUsageBitsColorTarget) != 0;
    nvrhiDesc.isUAV =
        (hgiDesc.usage & pxr::HgiTextureUsageBitsShaderWrite) != 0;
    nvrhiDesc.initialState =
        nvrhi::ResourceStates::Unknown;  // Assuming initial state is unknown
    return nvrhiDesc;
}

inline pxr::HgiTextureDesc ConvertToHgiTextureDesc(
    const nvrhi::TextureDesc& nvrhiDesc)
{
    pxr::HgiTextureDesc hgiDesc;
    hgiDesc.dimensions =
        pxr::GfVec3i(nvrhiDesc.width, nvrhiDesc.height, nvrhiDesc.depth);
    hgiDesc.layerCount = nvrhiDesc.arraySize;
    hgiDesc.mipLevels = nvrhiDesc.mipLevels;
    switch (nvrhiDesc.sampleCount) {
        case 1: hgiDesc.sampleCount = pxr::HgiSampleCount1; break;
        case 2: hgiDesc.sampleCount = pxr::HgiSampleCount2; break;
        case 4: hgiDesc.sampleCount = pxr::HgiSampleCount4; break;
        case 8: hgiDesc.sampleCount = pxr::HgiSampleCount8; break;
        case 16: hgiDesc.sampleCount = pxr::HgiSampleCount16; break;
        default:
            hgiDesc.sampleCount =
                pxr::HgiSampleCount1;  // Default to 1 if unknown
            break;
    }
    hgiDesc.format = ConvertToHgiFormat(nvrhiDesc.format);
    hgiDesc.type = (nvrhiDesc.dimension == nvrhi::TextureDimension::Texture3D)
                       ? pxr::HgiTextureType3D
                   : (nvrhiDesc.dimension == nvrhi::TextureDimension::Texture2D)
                       ? pxr::HgiTextureType2D
                       : pxr::HgiTextureType1D;
    hgiDesc.debugName = nvrhiDesc.debugName;
    hgiDesc.usage = 0;
    if (nvrhiDesc.isShaderResource)
        hgiDesc.usage |= pxr::HgiTextureUsageBitsShaderRead;
    if (nvrhiDesc.isRenderTarget)
        hgiDesc.usage |= pxr::HgiTextureUsageBitsColorTarget;
    if (nvrhiDesc.isUAV)
        hgiDesc.usage |= pxr::HgiTextureUsageBitsShaderWrite;
    hgiDesc.initialData = nullptr;  // Assuming no initial data
    return hgiDesc;
}
}  // namespace RHI

USTC_CG_NAMESPACE_CLOSE_SCOPE