#include <optix_device.h>

#include "../Optix/ShaderNameAbbre.h"
#include "params.h"

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

RGS(mesh)
{
    auto worldToClip = mesh_params.worldToClip;
    auto clipToWorld = worldToClip.get_inverse();

    uint3 dispatch_index = optixGetLaunchIndex();
    uint3 dispatch_dim = optixGetLaunchDimensions();

    float2 uv = make_float2(
        (dispatch_index.x + 0.5f) / dispatch_dim.x,
        (dispatch_index.y + 0.5f) / dispatch_dim.y);

    float3 origin = make_float3(0, 0, 0);
    float4 clip_rayend = make_float4(uv * 2.0f - 1.0f, 0.0f, 1.0f);

    float4 world_rayend = clipToWorld * clip;

    // optixTrace(
    //     params.handle,
    //     origin,
    //     dir,
    //     0,
    //     1e5f,
    //     1.0,
    //     OptixVisibilityMask(255),
    //     OPTIX_RAY_FLAG_NONE,
    //     0,
    //     1,
    //     0);
}

CHS(mesh)
{
}

MISS(mesh)
{
}

AHS(mesh)
{
}
