#pragma once

#include "USTC_CG.h"
#include <nvrhi/nvrhi.h>

USTC_CG_NAMESPACE_OPEN_SCOPE
int USTC_CG_API init(bool with_window = false, bool use_dx12 = false);
int USTC_CG_API shutdown();

nvrhi::IDevice* USTC_CG_API get_device();

USTC_CG_NAMESPACE_CLOSE_SCOPE
