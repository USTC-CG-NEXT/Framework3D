#pragma once
#include <nvrhi/nvrhi.h>
#include <nvrhi/utils.h>

template<typename DESC>
inline size_t gpu_resource_size(const DESC& handle)
{
    return 0;
}

template<>
inline size_t gpu_resource_size<nvrhi::TextureDesc>(
    const nvrhi::TextureDesc& desc)
{
    auto formatInfo = nvrhi::getFormatInfo(desc.format);
    return formatInfo.bytesPerBlock * formatInfo.blockSize * desc.width *
           desc.height * desc.arraySize;
}

template<>
inline size_t gpu_resource_size<nvrhi::BufferDesc>(
    const nvrhi::BufferDesc& desc)
{
    return desc.byteSize;
}

// Sampler
template<>
inline size_t gpu_resource_size<nvrhi::SamplerDesc>(
    const nvrhi::SamplerDesc& desc)
{
    return 0;
}
