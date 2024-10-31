#pragma once

#include <nvrhi/nvrhi.h>
#include <pxr/imaging/hgi/hgi.h>

#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace RHI {
inline pxr::HgiFormat ConvertToHgiFormat(nvrhi::Format format)
{
    switch (format) {
        case nvrhi::Format::R8_UINT: return pxr::HgiFormatR8Uint;
        case nvrhi::Format::R8_SINT: return pxr::HgiFormatR8Sint;
        case nvrhi::Format::R8_UNORM: return pxr::HgiFormatR8Unorm;
        case nvrhi::Format::R8_SNORM: return pxr::HgiFormatR8Snorm;
        case nvrhi::Format::RG8_UINT: return pxr::HgiFormatRG8Uint;
        case nvrhi::Format::RG8_SINT: return pxr::HgiFormatRG8Sint;
        case nvrhi::Format::RG8_UNORM: return pxr::HgiFormatRG8Unorm;
        case nvrhi::Format::RG8_SNORM: return pxr::HgiFormatRG8Snorm;
        case nvrhi::Format::R16_UINT: return pxr::HgiFormatR16Uint;
        case nvrhi::Format::R16_SINT: return pxr::HgiFormatR16Sint;
        case nvrhi::Format::R16_UNORM: return pxr::HgiFormatR16Unorm;
        case nvrhi::Format::R16_SNORM: return pxr::HgiFormatR16Snorm;
        case nvrhi::Format::R16_FLOAT: return pxr::HgiFormatR16Float;
        case nvrhi::Format::RGBA8_UINT: return pxr::HgiFormatRGBA8Uint;
        case nvrhi::Format::RGBA8_SINT: return pxr::HgiFormatRGBA8Sint;
        case nvrhi::Format::RGBA8_UNORM: return pxr::HgiFormatRGBA8Unorm;
        case nvrhi::Format::RGBA8_SNORM: return pxr::HgiFormatRGBA8Snorm;
        case nvrhi::Format::BGRA8_UNORM: return pxr::HgiFormatBGRA8Unorm;
        case nvrhi::Format::SRGBA8_UNORM: return pxr::HgiFormatSRGBA8Unorm;
        case nvrhi::Format::R16_FLOAT: return pxr::HgiFormatR16Float;
        case nvrhi::Format::R32_UINT: return pxr::HgiFormatR32Uint;
        case nvrhi::Format::R32_SINT: return pxr::HgiFormatR32Sint;
        case nvrhi::Format::R32_FLOAT: return pxr::HgiFormatR32Float;
        case nvrhi::Format::RGBA16_UINT: return pxr::HgiFormatRGBA16Uint;
        case nvrhi::Format::RGBA16_SINT: return pxr::HgiFormatRGBA16Sint;
        case nvrhi::Format::RGBA16_FLOAT: return pxr::HgiFormatRGBA16Float;
        case nvrhi::Format::RGBA32_UINT: return pxr::HgiFormatRGBA32Uint;
        case nvrhi::Format::RGBA32_SINT: return pxr::HgiFormatRGBA32Sint;
        case nvrhi::Format::RGBA32_FLOAT: return pxr::HgiFormatRGBA32Float;
        case nvrhi::Format::D16: return pxr::HgiFormatD16Unorm;
        case nvrhi::Format::D24S8: return pxr::HgiFormatD24UnormS8Uint;
        case nvrhi::Format::D32: return pxr::HgiFormatD32Float;
        case nvrhi::Format::D32S8: return pxr::HgiFormatD32FloatS8X24Uint;
        default: return pxr::HgiFormatInvalid;
    }
}

inline nvrhi::Format ConvertToNvrhiFormat(pxr::HgiFormat format)
{
    switch (format) {
        case pxr::HgiFormatR8Uint: return nvrhi::Format::R8_UINT;
        case pxr::HgiFormatR8Sint: return nvrhi::Format::R8_SINT;
        case pxr::HgiFormatR8Unorm: return nvrhi::Format::R8_UNORM;
        case pxr::HgiFormatR8Snorm: return nvrhi::Format::R8_SNORM;
        case pxr::HgiFormatRG8Uint: return nvrhi::Format::RG8_UINT;
        case pxr::HgiFormatRG8Sint: return nvrhi::Format::RG8_SINT;
        case pxr::HgiFormatRG8Unorm: return nvrhi::Format::RG8_UNORM;
        case pxr::HgiFormatRG8Snorm: return nvrhi::Format::RG8_SNORM;
        case pxr::HgiFormatR16Uint: return nvrhi::Format::R16_UINT;
        case pxr::HgiFormatR16Sint: return nvrhi::Format::R16_SINT;
        case pxr::HgiFormatR16Unorm: return nvrhi::Format::R16_UNORM;
        case pxr::HgiFormatR16Snorm: return nvrhi::Format::R16_SNORM;
        case pxr::HgiFormatR16Float: return nvrhi::Format::R16_FLOAT;
        case pxr::HgiFormatRGBA8Uint: return nvrhi::Format::RGBA8_UINT;
        case pxr::HgiFormatRGBA8Sint: return nvrhi::Format::RGBA8_SINT;
        case pxr::HgiFormatRGBA8Unorm: return nvrhi::Format::RGBA8_UNORM;
        case pxr::HgiFormatRGBA8Snorm: return nvrhi::Format::RGBA8_SNORM;
        case pxr::HgiFormatBGRA8Unorm: return nvrhi::Format::BGRA8_UNORM;
        case pxr::HgiFormatSRGBA8Unorm: return nvrhi::Format::SRGBA8_UNORM;
        case pxr::HgiFormatR16Float: return nvrhi::Format::R16_FLOAT;
        case pxr::HgiFormatR32Uint: return nvrhi::Format::R32_UINT;
        case pxr::HgiFormatR32Sint: return nvrhi::Format::R32_SINT;
        case pxr::HgiFormatR32Float: return nvrhi::Format::R32_FLOAT;
        case pxr::HgiFormatRGBA16Uint: return nvrhi::Format::RGBA16_UINT;
        case pxr::HgiFormatRGBA16Sint: return nvrhi::Format::RGBA16_SINT;
        case pxr::HgiFormatRGBA16Float: return nvrhi::Format::RGBA16_FLOAT;
        case pxr::HgiFormatRGBA32Uint: return nvrhi::Format::RGBA32_UINT;
        case pxr::HgiFormatRGBA32Sint: return nvrhi::Format::RGBA32_SINT;
        case pxr::HgiFormatRGBA32Float: return nvrhi::Format::RGBA32_FLOAT;
        case pxr::HgiFormatD16Unorm: return nvrhi::Format::D16;
        case pxr::HgiFormatD24UnormS8Uint: return nvrhi::Format::D24S8;
        case pxr::HgiFormatD32Float: return nvrhi::Format::D32;
        case pxr::HgiFormatD32FloatS8X24Uint: return nvrhi::Format::D32S8;
        default: return nvrhi::Format::UNKNOWN;
    }
}
}  // namespace RHI

USTC_CG_NAMESPACE_CLOSE_SCOPE