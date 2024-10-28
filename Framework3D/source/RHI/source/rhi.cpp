
#include <nvrhi/nvrhi.h>

#include <RHI/rhi.hpp>

#include "RHI/DeviceManager/DeviceManager.h"
#include "nvrhi/utils.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace rhi {

std::unique_ptr<DeviceManager> device_manager;

int init(bool with_window, bool use_dx12)
{
    device_manager.reset();
    auto api =
        use_dx12 ? nvrhi::GraphicsAPI::D3D12 : nvrhi::GraphicsAPI::VULKAN;
    device_manager = std::unique_ptr<DeviceManager>(DeviceManager::Create(api));

    DeviceCreationParameters params;

#ifdef _DEBUG
    //params.enableNvrhiValidationLayer = true;
    params.enableDebugRuntime = true;
#endif

    if (with_window) {
        return device_manager->CreateWindowDeviceAndSwapChain(
            params, "USTC_CG");
    }
    else {
        return device_manager->CreateHeadlessDevice(params);
    }
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

nvrhi::TextureHandle load_texture(
    const nvrhi::TextureDesc& desc,
    const void* data)
{
    nvrhi::IDevice* device = get_device();
    auto texture = device->createTexture(desc);

    if (data) {
        // Create a staging texture for uploading data
        nvrhi::TextureDesc stagingDesc = desc;
        stagingDesc.isRenderTarget = false;
        stagingDesc.isUAV = false;
        stagingDesc.initialState = nvrhi::ResourceStates::CopyDest;
        stagingDesc.keepInitialState = true;
        stagingDesc.debugName = "StagingTexture";

        auto stagingTexture = device->createStagingTexture(
            stagingDesc, nvrhi::CpuAccessMode::Write);

        size_t rowPitch;
        // Map the staging texture and copy data
        void* mappedData = device->mapStagingTexture(
            stagingTexture, {}, nvrhi::CpuAccessMode::Write, &rowPitch);
        if (mappedData) {
            const uint8_t* srcData = static_cast<const uint8_t*>(data);
            uint8_t* dstData = static_cast<uint8_t*>(mappedData);

            for (uint32_t y = 0; y < desc.height; ++y) {
                nvrhi::FormatInfo formatInfo = getFormatInfo(desc.format);
                auto bytesPerPixel =
                    formatInfo.bytesPerBlock * formatInfo.blockSize;
                memcpy(dstData, srcData, desc.width * bytesPerPixel);
                srcData += desc.width * bytesPerPixel;
                dstData += rowPitch;
            }

            device->unmapStagingTexture(stagingTexture);
        }

        // Copy data from the staging texture to the final texture
        nvrhi::CommandListHandle commandList = device->createCommandList();
        commandList->open();
        commandList->copyTexture(texture, {}, stagingTexture, {});
        commandList->close();
        device->executeCommandList(commandList);
    }
    return texture;
}

nvrhi::SamplerHandle sampler = nullptr;

DeviceManager* internal::get_device_manager()
{
    return device_manager.get();
}

int shutdown()
{
    sampler = nullptr;
    device_manager->Shutdown();
    device_manager.reset();
    return device_manager == nullptr;
}
}  // namespace rhi
USTC_CG_NAMESPACE_CLOSE_SCOPE