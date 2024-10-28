
#include <nvrhi/nvrhi.h>

#include <RHI/rhi.hpp>

#include "RHI/DeviceManager/DeviceManager.h"

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
    params.enableNvrhiValidationLayer = true;
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

DeviceManager* internal::get_device_manager()
{
    return device_manager.get();
}

int shutdown()
{
    device_manager->Shutdown();
    device_manager.reset();
    return device_manager == nullptr;
}
}  // namespace rhi
USTC_CG_NAMESPACE_CLOSE_SCOPE