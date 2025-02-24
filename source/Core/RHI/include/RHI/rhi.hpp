#pragma once

#include <nvrhi/nvrhi.h>

#include <map>
#include <string>

#include "RHI/api.h"

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
RHI_API std::tuple<nvrhi::TextureHandle, nvrhi::StagingTextureHandle>
load_texture(const nvrhi::TextureDesc& desc, const void* data);

RHI_API void write_texture(
    nvrhi::ITexture* texture,
    nvrhi::IStagingTexture* staging,
    const void* data);

RHI_API void copy_from_texture(
    nvrhi::TextureHandle& texture,
    nvrhi::ITexture* source);
struct nvrhi_image {
    nvrhi::TextureDesc nvrhi_desc = {};
    nvrhi::TextureHandle nvrhi_texture = nullptr;
    nvrhi::StagingTextureHandle nvrhi_staging = nullptr;
    nvrhi::Format present_format = nvrhi::Format::RGBA8_UNORM;
};

extern RHI_API std::map<std::string, nvrhi_image> rhi_images;

template<typename T>
auto rhi_imgui_image(
    std::string name,
    const std::vector<T>& buffer,
    int texture_width,
    int texture_height,
    int width,
    int height,
    nvrhi::Format format)
{
    if (rhi_images.find(name) == rhi_images.end()) {
        rhi_images[name] = {};
        rhi_images[name].nvrhi_desc.width = texture_width;
        rhi_images[name].nvrhi_desc.height = texture_height;
        rhi_images[name].nvrhi_desc.format = format;
        rhi_images[name].nvrhi_desc.debugName = name.c_str();
        std::tie(
            rhi_images[name].nvrhi_texture, rhi_images[name].nvrhi_staging) =
            load_texture(rhi_images[name].nvrhi_desc, buffer.data());
    }
    else {
        // Check if the texture size changed
        if (rhi_images[name].nvrhi_desc.width != texture_width ||
            rhi_images[name].nvrhi_desc.height != texture_height) {
            rhi_images[name].nvrhi_desc.width = texture_width;
            rhi_images[name].nvrhi_desc.height = texture_height;
            rhi_images[name].nvrhi_desc.format = format;
            rhi_images[name].nvrhi_desc.debugName = name.c_str();

            auto old_texture = rhi_images[name].nvrhi_texture;
            auto old_staging = rhi_images[name].nvrhi_staging;

            std::tie(
                rhi_images[name].nvrhi_texture,
                rhi_images[name].nvrhi_staging) =
                load_texture(rhi_images[name].nvrhi_desc, buffer.data());

            // old_texture = nullptr;
            // old_staging = nullptr;
        }
        else {
            write_texture(
                rhi_images[name].nvrhi_texture.Get(),
                rhi_images[name].nvrhi_staging.Get(),
                buffer.data());
        }
    }

    return rhi_images[name].nvrhi_texture.Get();
}

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
