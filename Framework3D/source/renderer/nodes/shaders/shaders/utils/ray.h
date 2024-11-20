#ifndef RAY_H
#define RAY_H

#include "cpp_shader_macro.h"

#ifdef __cplusplus
#include "Spectrum.slang"
#else
import utils.Spectrum;
#endif

// Constants
struct RayInfo {
#ifdef __cplusplus
    USING_PXR_MATH_TYPES
#endif
    float3 Origin;
    float TMin;
    float3 Direction;
    float TMax;
    RGBSpectrum throughput;
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
RayInfo transformRay(RayInfo ray, float4x4 transform)
{
    RayInfo result;
    result.Origin = mul(float4(ray.Origin, 1), transform).xyz;
    result.TMin = ray.TMin;
    result.Direction = mul(float4(ray.Direction, 0), transform).xyz;
    result.TMax = ray.TMax;
    result.throughput = ray.throughput;
    return result;
}
#endif
#endif