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
    auto rays = mesh_params.rays[GetLaunchID()];

    //auto uv_center = (patch.uv0 + patch.uv1 + patch.uv2 + patch.uv3) / 4.0f;

    //float3 origin = make_float3(uv_center.x, uv_center.y, 10000.0f);

    //float3 dir = make_float3(0, 0, -1);

    //optixTrace(
    //    params.handle,
    //    origin,
    //    dir,
    //    0,
    //    1e5f,
    //    1.0,
    //    OptixVisibilityMask(255),
    //    OPTIX_RAY_FLAG_NONE,
    //    0,
    //    1,
    //    0);
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
