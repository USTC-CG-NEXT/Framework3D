#include <nvrhi/nvrhi.h>

#include <RHI/rhi.hpp>

#include "RHI/DeviceManager/DeviceManager.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

std::unique_ptr<DeviceManager> device_manager;

int init(bool with_window, bool use_dx12)
{
    device_manager.reset();
    auto api =
        use_dx12 ? nvrhi::GraphicsAPI::D3D12 : nvrhi::GraphicsAPI::VULKAN;
    device_manager = std::unique_ptr<DeviceManager>(DeviceManager::Create(api));

    DeviceCreationParameters params;

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

int shutdown()
{
    device_manager.reset();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE