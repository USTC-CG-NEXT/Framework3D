#pragma once
#include "cpp_shader_macro.h"
#ifndef __cplusplus
import utils.ray;
#else
#include "utils/ray.slang"
#endif

struct HitObjectInfo {
#ifdef __cplusplus
    USING_PXR_MATH_TYPES
#endif
    uint InstanceIndex;
    uint GeometryIndex;
    uint PrimitiveIndex;
    uint HitKind;
    uint RayContributionToHitGroupIndex;
    uint MultiplierForGeometryContributionToHitGroupIndex;
    float2 attributes;
    RayInfo rays;
};