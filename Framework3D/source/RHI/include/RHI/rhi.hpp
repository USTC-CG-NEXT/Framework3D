#pragma once

#include <nvrhi/nvrhi.h>

#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class DeviceManager;

namespace rhi {

USTC_CG_API int init(bool with_window = false, bool use_dx12 = false);
USTC_CG_API int shutdown();

USTC_CG_API nvrhi::IDevice* get_device();

namespace internal {
    USTC_CG_API DeviceManager* get_device_manager();

}

}  // namespace rhi
USTC_CG_NAMESPACE_CLOSE_SCOPE
