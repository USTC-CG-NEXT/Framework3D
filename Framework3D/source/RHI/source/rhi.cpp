
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

nvrhi::TextureHandle load_texture(
    const nvrhi::TextureDesc& desc,
    const void* data)
{
    nvrhi::IDevice* device = get_device();
    auto texture = device->createTexture(desc);

    
}

nvrhi::BindingSetHandle create_texture_binding_set(nvrhi::ITexture* texture)
{
    nvrhi::IDevice* device = get_device();
    nvrhi::SamplerHandle sampler = device->createSampler(nvrhi::SamplerDesc());
    nvrhi::BindingSetDesc desc;
    desc.bindings = { nvrhi::BindingSetItem::Sampler() };


    desc.bindings[0].resource = texture;
    return device->createBindingSet(desc);
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