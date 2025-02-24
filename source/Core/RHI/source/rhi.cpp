
#include <nvrhi/nvrhi.h>

#include <RHI/internal/nvrhi_equality.hpp>
#include <RHI/rhi.hpp>
#include <memory>

#include "RHI/DeviceManager/DeviceManager.h"
#include "nvrhi/utils.h"
#include "pxr/imaging/garch/glApi.h"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace RHI {

std::unique_ptr<DeviceManager> device_manager = nullptr;
std::map<std::string, nvrhi_image> rhi_images{};

int init(bool with_window, bool use_dx12)
{
    if (device_manager) {
        log::warning("Trying to initialize the RHI again");
        return 0;
    }

    auto api =
        use_dx12 ? nvrhi::GraphicsAPI::D3D12 : nvrhi::GraphicsAPI::VULKAN;
    device_manager = std::unique_ptr<DeviceManager>(DeviceManager::Create(api));

    DeviceCreationParameters params;

    params.enableRayTracingExtensions = true;
    params.enableComputeQueue = true;
    params.enableCopyQueue = true;
    //params.adapterIndex = 0;

    params.optionalVulkanInstanceExtensions = {
        VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME
    };
    params.optionalVulkanDeviceExtensions = {
        VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
        VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
        VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
        VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME,
        VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME,
        VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME
#else
        VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME
#endif
    };

    params.swapChainFormat = nvrhi::Format::RGBA8_UNORM;
#ifdef _DEBUG
    // params.enableNvrhiValidationLayer = true;
    params.enableDebugRuntime = true;
#endif

    if (with_window) {
        auto ret =
            device_manager->CreateWindowDeviceAndSwapChain(params, "USTC_CG");

        device_manager->m_callbacks.afterPresent = [](DeviceManager& manager) {
            manager.SetInformativeWindowTitle("USTC_CG");
        };

        return ret;
    }
    else {
        return device_manager->CreateHeadlessDevice(params);
    }
    return 1;
}

nvrhi::IDevice* get_device()
{
    if (!device_manager) {
        init();
    }
    return device_manager->GetDevice();
}

nvrhi::GraphicsAPI get_backend()
{
    return get_device()->getGraphicsAPI();
}
size_t calculate_bytes_per_pixel(nvrhi::Format format)
{
    nvrhi::FormatInfo formatInfo = getFormatInfo(format);
    return formatInfo.bytesPerBlock * formatInfo.blockSize;
}

void write_texture(
    nvrhi::ITexture* texture,
    nvrhi::IStagingTexture* staging,
    const void* data)
{
    nvrhi::IDevice* device = get_device();
    size_t rowPitch;
    void* mappedData = device->mapStagingTexture(
        staging, {}, nvrhi::CpuAccessMode::Write, &rowPitch);
    if (mappedData) {
        const uint8_t* srcData = static_cast<const uint8_t*>(data);
        uint8_t* dstData = static_cast<uint8_t*>(mappedData);

        for (uint32_t y = 0; y < texture->getDesc().height; ++y) {
            auto bytesPerPixel =
                calculate_bytes_per_pixel(texture->getDesc().format);
            memcpy(dstData, srcData, texture->getDesc().width * bytesPerPixel);
            srcData += texture->getDesc().width * bytesPerPixel;
            dstData += rowPitch;
        }

        device->unmapStagingTexture(staging);
    }

    nvrhi::CommandListHandle commandList = device->createCommandList();
    commandList->open();
    commandList->copyTexture(texture, {}, staging, {});
    commandList->close();
    device->executeCommandList(commandList);
}

std::tuple<nvrhi::TextureHandle, nvrhi::StagingTextureHandle> load_texture(
    const nvrhi::TextureDesc& desc,
    const void* data)
{
    nvrhi::IDevice* device = get_device();
    auto texture = device->createTexture(desc);
    // Create a staging texture for uploading data
    nvrhi::TextureDesc stagingDesc = desc;
    stagingDesc.isRenderTarget = false;
    stagingDesc.isUAV = false;
    stagingDesc.initialState = nvrhi::ResourceStates::CopyDest;
    stagingDesc.keepInitialState = true;
    stagingDesc.debugName = "StagingTexture";

    auto stagingTexture =
        device->createStagingTexture(stagingDesc, nvrhi::CpuAccessMode::Write);

    write_texture(texture, stagingTexture, data);
    assert(texture);
    return std::make_tuple(texture, stagingTexture);
}

inline void copy_from_texture(
    nvrhi::TextureHandle& texture,
    nvrhi::ITexture* source)
{
    nvrhi::IDevice* device = get_device();
    nvrhi::TextureDesc desc = source->getDesc();
    if (!texture || texture->getDesc() != source->getDesc()) {
        texture = device->createTexture(desc);
    }

    nvrhi::CommandListHandle commandList = device->createCommandList();
    commandList->open();
    commandList->copyTexture(texture, {}, source, {});
    commandList->close();
    device->executeCommandList(commandList);
}

nvrhi::TextureHandle load_ogl_texture(
    const nvrhi::TextureDesc& desc,
    unsigned gl_texture)
{
    auto device = RHI::get_device();
    vk::Device vk_device =
        VkDevice(device->getNativeObject(nvrhi::ObjectTypes::VK_Device));
    vk::PhysicalDevice vk_physical_device = VkPhysicalDevice(
        device->getNativeObject(nvrhi::ObjectTypes::VK_PhysicalDevice));

    // Get the OpenGL texture handle
    GLuint64 glHandle = glGetTextureHandleARB(gl_texture);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
        return nullptr;
    }

    // Create Vulkan image with external memory
    vk::ImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
    imageCreateInfo.extent.width = desc.width;
    imageCreateInfo.extent.height = desc.height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = desc.mipLevels;
    imageCreateInfo.arrayLayers = desc.arraySize;
    imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imageCreateInfo.usage = vk::ImageUsageFlagBits::eSampled;
    imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

    // Specify external memory handle types
    vk::ExternalMemoryImageCreateInfo externalMemoryInfo = {};
    externalMemoryInfo.handleTypes =
        vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;

    imageCreateInfo.pNext = &externalMemoryInfo;

    // Create the Vulkan image
    vk::Image vkImage = vk_device.createImage(imageCreateInfo);

    // Get memory requirements
    vk::MemoryRequirements memRequirements =
        vk_device.getImageMemoryRequirements(vkImage);

    // Set up memory allocation info with imported handle
    vk::MemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.allocationSize = memRequirements.size;

    uint32_t memoryTypeIndex = 0;
    vk::PhysicalDeviceMemoryProperties memoryProperties =
        vk_physical_device.getMemoryProperties();
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags &
             vk::MemoryPropertyFlagBits::eDeviceLocal)) {
            memoryTypeIndex = i;
            break;
        }
    }
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

#if defined(_WIN32)
    vk::ImportMemoryWin32HandleInfoKHR importMemoryInfo = {};
    importMemoryInfo.handleType =
        vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
    importMemoryInfo.handle = reinterpret_cast<HANDLE>(glHandle);

    memoryAllocateInfo.pNext = &importMemoryInfo;
#else
    vk::ImportMemoryFdInfoKHR importMemoryInfo = {};
    importMemoryInfo.handleType =
        vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd;
    importMemoryInfo.fd = static_cast<int>(glHandle);

    memoryAllocateInfo.pNext = &importMemoryInfo;
#endif

    // Allocate memory
    vk::DeviceMemory vkMemory = vk_device.allocateMemory(memoryAllocateInfo);

    // Bind memory to the image
    vk_device.bindImageMemory(vkImage, vkMemory, 0);

    // Create NVRHI texture handle
    nvrhi::TextureHandle texture = device->createHandleForNativeTexture(
        nvrhi::ObjectTypes::VK_Image, static_cast<VkImage>(vkImage), desc);

    return texture;
}

DeviceManager* internal::get_device_manager()
{
    return device_manager.get();
}

int shutdown()
{
    std::map<std::string, nvrhi_image>().swap(rhi_images);
    device_manager->Shutdown();
    device_manager.reset();
    return device_manager == nullptr;
}
}  // namespace RHI
USTC_CG_NAMESPACE_CLOSE_SCOPE