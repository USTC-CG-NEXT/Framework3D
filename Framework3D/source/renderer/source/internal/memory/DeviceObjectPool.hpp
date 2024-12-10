#pragma once
#include <RHI/rhi.hpp>
#include <unordered_map>

#include "../../api.h"
#include "DeviceMemoryPool.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE

template<typename T>
class DeviceObjectPool : public DeviceMemoryPool<T> {
   public:
};

USTC_CG_NAMESPACE_CLOSE_SCOPE