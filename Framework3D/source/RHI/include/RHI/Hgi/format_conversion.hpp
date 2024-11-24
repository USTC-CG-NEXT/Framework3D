#pragma once

#include <nvrhi/nvrhi.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgi/types.h>

#include "RHI/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace RHI {

inline pxr::HgiFormat ConvertToHgiFormat(nvrhi::Format format)
{
    switch (format) {
        case nvrhi::Format::R8_UNORM: return pxr::HgiFormatUNorm8;
        case nvrhi::Format::RG8_UNORM: return pxr::HgiFormatUNorm8Vec2;
        case nvrhi::Format::RGBA8_UNORM: return pxr::HgiFormatUNorm8Vec4;
        case nvrhi::Format::R16_FLOAT: return pxr::HgiFormatFloat16;
        case nvrhi::Format::RG16_FLOAT: return pxr::HgiFormatFloat16Vec2;
        case nvrhi::Format::RGBA16_FLOAT: return pxr::HgiFormatFloat16Vec4;
        case nvrhi::Format::R32_FLOAT: return pxr::HgiFormatFloat32;
        case nvrhi::Format::RG32_FLOAT: return pxr::HgiFormatFloat32Vec2;
        case nvrhi::Format::RGB32_FLOAT: return pxr::HgiFormatFloat32Vec3;
        case nvrhi::Format::RGBA32_FLOAT: return pxr::HgiFormatFloat32Vec4;
        case nvrhi::Format::R16_UINT: return pxr::HgiFormatUInt16;
        case nvrhi::Format::RG16_UINT: return pxr::HgiFormatUInt16Vec2;
        case nvrhi::Format::RGBA16_UINT: return pxr::HgiFormatUInt16Vec4;
        case nvrhi::Format::R32_UINT: return pxr::HgiFormatInt32;
        case nvrhi::Format::RG32_UINT: return pxr::HgiFormatInt32Vec2;
        case nvrhi::Format::RGB32_UINT: return pxr::HgiFormatInt32Vec3;
        case nvrhi::Format::RGBA32_UINT: return pxr::HgiFormatInt32Vec4;
        case nvrhi::Format::SRGBA8_UNORM: return pxr::HgiFormatUNorm8Vec4srgb;
        case nvrhi::Format::BC6H_SFLOAT: return pxr::HgiFormatBC6FloatVec3;
        case nvrhi::Format::BC6H_UFLOAT: return pxr::HgiFormatBC6UFloatVec3;
        case nvrhi::Format::BC7_UNORM: return pxr::HgiFormatBC7UNorm8Vec4;
        case nvrhi::Format::BC7_UNORM_SRGB:
            return pxr::HgiFormatBC7UNorm8Vec4srgb;
        case nvrhi::Format::BC1_UNORM: return pxr::HgiFormatBC1UNorm8Vec4;
        case nvrhi::Format::BC3_UNORM: return pxr::HgiFormatBC3UNorm8Vec4;
        case nvrhi::Format::D32: return pxr::HgiFormatFloat32;
        case nvrhi::Format::D32S8: return pxr::HgiFormatFloat32UInt8;
        default: return pxr::HgiFormatInvalid;
    }
}

inline nvrhi::Format ConvertToNvrhiFormat(pxr::HgiFormat format)
{
    switch (format) {
        case pxr::HgiFormatUNorm8: return nvrhi::Format::R8_UNORM;
        case pxr::HgiFormatUNorm8Vec2: return nvrhi::Format::RG8_UNORM;
        case pxr::HgiFormatUNorm8Vec4: return nvrhi::Format::RGBA8_UNORM;
        case pxr::HgiFormatFloat16: return nvrhi::Format::R16_FLOAT;
        case pxr::HgiFormatFloat16Vec2: return nvrhi::Format::RG16_FLOAT;
        case pxr::HgiFormatFloat16Vec4: return nvrhi::Format::RGBA16_FLOAT;
        case pxr::HgiFormatFloat32: return nvrhi::Format::R32_FLOAT;
        case pxr::HgiFormatFloat32Vec2: return nvrhi::Format::RG32_FLOAT;
        case pxr::HgiFormatFloat32Vec3: return nvrhi::Format::RGB32_FLOAT;
        case pxr::HgiFormatFloat32Vec4: return nvrhi::Format::RGBA32_FLOAT;
        case pxr::HgiFormatUInt16: return nvrhi::Format::R16_UINT;
        case pxr::HgiFormatUInt16Vec2: return nvrhi::Format::RG16_UINT;
        case pxr::HgiFormatUInt16Vec4: return nvrhi::Format::RGBA16_UINT;
        case pxr::HgiFormatInt32: return nvrhi::Format::R32_UINT;
        case pxr::HgiFormatInt32Vec2: return nvrhi::Format::RG32_UINT;
        case pxr::HgiFormatInt32Vec3: return nvrhi::Format::RGB32_UINT;
        case pxr::HgiFormatInt32Vec4: return nvrhi::Format::RGBA32_UINT;
        case pxr::HgiFormatUNorm8Vec4srgb: return nvrhi::Format::SRGBA8_UNORM;
        case pxr::HgiFormatBC6FloatVec3: return nvrhi::Format::BC6H_SFLOAT;
        case pxr::HgiFormatBC6UFloatVec3: return nvrhi::Format::BC6H_UFLOAT;
        case pxr::HgiFormatBC7UNorm8Vec4: return nvrhi::Format::BC7_UNORM;
        case pxr::HgiFormatBC7UNorm8Vec4srgb:
            return nvrhi::Format::BC7_UNORM_SRGB;
        case pxr::HgiFormatBC1UNorm8Vec4: return nvrhi::Format::BC1_UNORM;
        case pxr::HgiFormatBC3UNorm8Vec4: return nvrhi::Format::BC3_UNORM;
        // case pxr::HgiFormatFloat32: return nvrhi::Format::D32;
        case pxr::HgiFormatFloat32UInt8: return nvrhi::Format::D32S8;
        default: return nvrhi::Format::UNKNOWN;
    }
}

inline nvrhi::Format ConvertToNvrhiFormat(pxr::HdFormat format)
{
    using namespace pxr;
    switch (format) {
        case HdFormatUNorm8: return nvrhi::Format::R8_UNORM;
        case HdFormatUNorm8Vec2: return nvrhi::Format::RG8_UNORM;
        case HdFormatUNorm8Vec4: return nvrhi::Format::RGBA8_UNORM;
        case HdFormatFloat16: return nvrhi::Format::R16_FLOAT;
        case HdFormatFloat16Vec2: return nvrhi::Format::RG16_FLOAT;
        case HdFormatFloat16Vec4: return nvrhi::Format::RGBA16_FLOAT;
        case HdFormatFloat32: return nvrhi::Format::R32_FLOAT;
        case HdFormatFloat32Vec2: return nvrhi::Format::RG32_FLOAT;
        case HdFormatFloat32Vec3: return nvrhi::Format::RGB32_FLOAT;
        case HdFormatFloat32Vec4: return nvrhi::Format::RGBA32_FLOAT;
        case HdFormatUInt16: return nvrhi::Format::R16_UINT;
        case HdFormatUInt16Vec2: return nvrhi::Format::RG16_UINT;
        case HdFormatUInt16Vec4: return nvrhi::Format::RGBA16_UINT;
        case HdFormatInt32: return nvrhi::Format::R32_UINT;
        case HdFormatInt32Vec2: return nvrhi::Format::RG32_UINT;
        case HdFormatInt32Vec3: return nvrhi::Format::RGB32_UINT;
        case HdFormatInt32Vec4: return nvrhi::Format::RGBA32_UINT;
        case HdFormatFloat32UInt8: return nvrhi::Format::D32S8;
        default: return nvrhi::Format::UNKNOWN;
    }
}

inline pxr::HdFormat ConvertToHdFormat(nvrhi::Format format)
{
    using namespace pxr;
    switch (format) {
        case nvrhi::Format::R8_UNORM: return HdFormatUNorm8;
        case nvrhi::Format::RG8_UNORM: return HdFormatUNorm8Vec2;
        case nvrhi::Format::RGBA8_UNORM: return HdFormatUNorm8Vec4;
        case nvrhi::Format::R16_FLOAT: return HdFormatFloat16;
        case nvrhi::Format::RG16_FLOAT: return HdFormatFloat16Vec2;
        case nvrhi::Format::RGBA16_FLOAT: return HdFormatFloat16Vec4;
        case nvrhi::Format::R32_FLOAT: return HdFormatFloat32;
        case nvrhi::Format::RG32_FLOAT: return HdFormatFloat32Vec2;
        case nvrhi::Format::RGB32_FLOAT: return HdFormatFloat32Vec3;
        case nvrhi::Format::RGBA32_FLOAT: return HdFormatFloat32Vec4;
        case nvrhi::Format::R16_UINT: return HdFormatUInt16;
        case nvrhi::Format::RG16_UINT: return HdFormatUInt16Vec2;
        case nvrhi::Format::RGBA16_UINT: return HdFormatUInt16Vec4;
        case nvrhi::Format::R32_UINT: return HdFormatInt32;
        case nvrhi::Format::RG32_UINT: return HdFormatInt32Vec2;
        case nvrhi::Format::RGB32_UINT: return HdFormatInt32Vec3;
        case nvrhi::Format::RGBA32_UINT: return HdFormatInt32Vec4;
        case nvrhi::Format::D32S8: return HdFormatFloat32UInt8;
        default: return HdFormatInvalid;
    }
}

}  // namespace RHI

USTC_CG_NAMESPACE_CLOSE_SCOPE