#pragma once

#include "cpp_shader_macro.h"
// Constants
struct RayInfo {
#ifdef __cplusplus
    USING_PXR_MATH_TYPES
#endif
    float3 Origin;
    float TMin;
    float3 Direction;
    float TMax;
};

#ifndef __cplusplus
RayDesc get_ray_desc(RayInfo info)
{
    RayDesc ray_desc;
    ray_desc.Direction = info.Direction;
    ray_desc.Origin = info.Origin;
    ray_desc.TMin = info.TMin;
    ray_desc.TMax = info.TMax;
    return ray_desc;
}

#endif
