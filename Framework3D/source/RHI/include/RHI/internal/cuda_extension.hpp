#pragma once
#include <RHI/api.h>
#include <nvrhi/nvrhi.h>

#include <cstdint>
#include <string>

USTC_CG_NAMESPACE_OPEN_SCOPE

namespace cuda {
RHI_API int cuda_init();
RHI_API int optix_init();
RHI_API int cuda_shutdown();
}  // namespace cuda

USTC_CG_NAMESPACE_CLOSE_SCOPE