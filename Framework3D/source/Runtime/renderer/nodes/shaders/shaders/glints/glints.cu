#include <optix_device.h>

#include "../Optix/ShaderNameAbbre.h"
#include "glints_params.h"

inline unsigned GetLaunchID()
{
    uint3 launch_index = optixGetLaunchIndex();
    return launch_index.x;
}

__device__ float2 operator+(const float2& a, const float2& b)
{
    return make_float2(a.x + b.x, a.y + b.y);
}

__device__ float2 operator/(const float2& a, const float b)
{
    return make_float2(a.x / b, a.y / b);
}

RGS(line)
{
    auto patch = params.patches[GetLaunchID()];

    auto uv_center = (patch.uv0 + patch.uv1 + patch.uv2 + patch.uv3) / 4.0f;

    float3 origin = make_float3(uv_center.x, uv_center.y, 100.0f);

    float3 dir = make_float3(0, 0, -1);

    optixTrace(
        params.handle,
        origin,
        dir,
        0,
        200.f,
        1.0,
        OptixVisibilityMask(255),
        OPTIX_RAY_FLAG_NONE,
        0,
        1,
        0);
}

CHS(line)
{
}

MISS(line)
{
}

AHS(line)
{
    auto lineid = optixGetPrimitiveIndex();
    params.patch_line_pairs->Push({ lineid, GetLaunchID() });
    optixIgnoreIntersection();
}
