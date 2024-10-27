#pragma once
#include <nvrhi/nvrhi.h>

#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

int USTC_CG_API init();
int USTC_CG_API shutdown();

nvrhi::DeviceHandle USTC_CG_API get_device();

USTC_CG_NAMESPACE_CLOSE_SCOPE
