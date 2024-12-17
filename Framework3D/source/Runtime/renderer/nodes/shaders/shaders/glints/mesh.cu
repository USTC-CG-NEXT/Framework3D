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

__device__ float2 operator/(const float2& a, const float2& b)
{
    return make_float2(a.x / b.x, a.y / b.y);
}

__device__ float3 operator+(const float3& a, const float3& b)
{
    return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

__device__ float3 operator-(const float3& a, const float3& b)
{
    return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

__device__ float3 normalize(const float3& v)
{
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return make_float3(v.x / length, v.y / length, v.z / length);
}

__device__ float3 make_float3(const float4& a)
{
    return make_float3(a.x, a.y, a.z);
}

__device__ float4 operator/=(float4& a, const float b)
{
    a.x /= b;
    a.y /= b;
    a.z /= b;
    a.w /= b;
    return a;
}

RGS(mesh)
{
    uint3 launch_index = optixGetLaunchIndex();
    uint3 launch_dimensions = optixGetLaunchDimensions();

    float2 pixel_position_f = make_float2(launch_index.x, launch_index.y);

    float2 uv = pixel_position_f /
                make_float2(launch_dimensions.x, launch_dimensions.y);
    float4 clip_pos =
        make_float4(uv.x * 2.0f - 1.0f, uv.y * 2.0f - 1.0f, 1.0f, 1.0f);

    auto clipToWorld = mesh_params.worldToClip.get_inverse();

    float4 view_pos = clipToWorld * clip_pos;
    view_pos /= view_pos.w;

    float3 origin = make_float3(0, 0, 0);

    float3 camera_right = make_float3(1, 0, 0);
    float3 camera_up = make_float3(0, 1, 0);

    float3 direction = normalize(make_float3(view_pos) - origin);

    optixTrace(
        mesh_params.handle,
        origin,
        direction,
        0.0f,
        1e5f,
        1.0f,
        OptixVisibilityMask(255),
        OPTIX_RAY_FLAG_NONE,
        0,
        1,
        0);
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
