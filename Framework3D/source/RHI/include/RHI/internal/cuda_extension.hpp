#pragma once
#include <RHI/api.h>
#include <nvrhi/nvrhi.h>

#include <cstdint>
#include <string>

USTC_CG_NAMESPACE_OPEN_SCOPE

RHI_API void cuda_init();
RHI_API void cuda_shutdown();

USTC_CG_NAMESPACE_CLOSE_SCOPE