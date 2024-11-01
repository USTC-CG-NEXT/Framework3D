#pragma once

#include <nvrhi/nvrhi.h>

#include "rhi/api.h"

namespace vk {
class Image;
}

USTC_CG_NAMESPACE_OPEN_SCOPE
class DeviceManager;

namespace RHI {

RHI_API int init(bool with_window = false, bool use_dx12 = false);
RHI_API int shutdown();

RHI_API nvrhi::IDevice* get_device();
RHI_API nvrhi::GraphicsAPI get_backend();
RHI_API size_t calculate_bytes_per_pixel(nvrhi::Format format);
RHI_API nvrhi::TextureHandle load_texture(
    const nvrhi::TextureDesc& desc,
    const void* data);


/**
 * 
 * @param desc Not tested. Don't use!
 * @param gl_texture 
 * @return 
 */
RHI_API nvrhi::TextureHandle load_ogl_texture(
    const nvrhi::TextureDesc& desc,
    unsigned gl_texture);

namespace internal {
    RHI_API DeviceManager* get_device_manager();

}

}  // namespace RHI
USTC_CG_NAMESPACE_CLOSE_SCOPE
