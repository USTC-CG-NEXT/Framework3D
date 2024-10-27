#include <nvrhi/nvrhi.h>

#include <RHI/rhi.hpp>

#include "nvrhi/vulkan.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
nvrhi::DeviceHandle device;

int init()
{
    nvrhi::vulkan::DeviceDesc desc;
    device = createDevice(desc);

    return device != nullptr;
}

nvrhi::DeviceHandle get_device()
{
    return device;
}

int shutdown()
{
    device = nullptr;
    return device == nullptr;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE