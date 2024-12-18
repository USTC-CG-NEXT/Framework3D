#include <optix_device.h>
#include <vector_functions.h>
#include <vector_types.h>

#include "../Optix/ShaderNameAbbre.h"
#include "mesh_params.h"

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

__device__ float4 make_float4(const float3& a, const float b)
{
    return make_float4(a.x, a.y, a.z, b);
}

__device__ float4 operator/=(float4& a, const float b)
{
    a.x /= b;
    a.y /= b;
    a.z /= b;
    a.w /= b;
    return a;
}

__device__ float4 operator/(const float4& a, const float b)
{
    return make_float4(a.x / b, a.y / b, a.z / b, a.w / b);
}

struct Payload {
    float2 uv;
    float3 corner0;
    float3 corner1;
    float3 corner2;
    unsigned hit;

    void set_self()
    {
        optixSetPayload_0(__float_as_uint(uv.x));
        optixSetPayload_1(__float_as_uint(uv.y));
        optixSetPayload_2(__float_as_uint(corner0.x));
        optixSetPayload_3(__float_as_uint(corner0.y));
        optixSetPayload_4(__float_as_uint(corner0.z));
        optixSetPayload_5(__float_as_uint(corner1.x));
        optixSetPayload_6(__float_as_uint(corner1.y));
        optixSetPayload_7(__float_as_uint(corner1.z));
        optixSetPayload_8(__float_as_uint(corner2.x));
        optixSetPayload_9(__float_as_uint(corner2.y));
        optixSetPayload_10(__float_as_uint(corner2.z));
        optixSetPayload_11(hit);
    }
};
#define Payload_As_Params(payload_name)                          \
    reinterpret_cast<unsigned int&>(payload_name.uv.x),          \
        reinterpret_cast<unsigned int&>(payload_name.uv.y),      \
        reinterpret_cast<unsigned int&>(payload_name.corner0.x), \
        reinterpret_cast<unsigned int&>(payload_name.corner0.y), \
        reinterpret_cast<unsigned int&>(payload_name.corner0.z), \
        reinterpret_cast<unsigned int&>(payload_name.corner1.x), \
        reinterpret_cast<unsigned int&>(payload_name.corner1.y), \
        reinterpret_cast<unsigned int&>(payload_name.corner1.z), \
        reinterpret_cast<unsigned int&>(payload_name.corner2.x), \
        reinterpret_cast<unsigned int&>(payload_name.corner2.y), \
        reinterpret_cast<unsigned int&>(payload_name.corner2.z), \
        payload_name.hit

__device__ void calculateRayParameters(
    const uint3& launch_index,
    const uint3& launch_dimensions,
    float bias_x,
    float bias_y,
    float3& origin,
    float3& direction)
{
    float2 pixel_position_f =
        make_float2(launch_index.x + bias_x, launch_index.y + bias_y);
    float2 uv = pixel_position_f /
                make_float2(launch_dimensions.x, launch_dimensions.y);
    float4 clip_pos =
        make_float4(uv.x * 2.0f - 1.0f, uv.y * 2.0f - 1.0f, 1.0f, 1.0f);

    origin = make_float3(0, 0, 0);
    direction = normalize(make_float3(clip_pos) - origin);

    auto clipToWorld = mesh_params.worldToClip.get_inverse();
    float4 world_origin = clipToWorld * make_float4(0, 0, 0, 1);
    origin = make_float3(world_origin / world_origin.w);
    direction = make_float3(clipToWorld * make_float4(direction, 0.f));
    direction = normalize(direction);
}

RGS(mesh)
{
    uint3 launch_index = optixGetLaunchIndex();
    uint3 launch_dimensions = optixGetLaunchDimensions();

    float bias_x = 0.5f;
    float bias_y = 0.5f;

    float3 origin;
    float3 direction;

    Payload payload;
    payload.hit = false;

    calculateRayParameters(
        launch_index, launch_dimensions, bias_x, bias_y, origin, direction);

    optixTrace(
        mesh_params.handle,
        origin,
        direction,
        0.0f,
        1e5f,
        1.0f,
        OptixVisibilityMask(255),
        unsigned(OPTIX_RAY_FLAG_NONE),
        unsigned(0),
        unsigned(1),
        unsigned(0),
        Payload_As_Params(payload));

    if (payload.hit) {
        Patch patch;

        calculateRayParameters(
            launch_index, launch_dimensions, 0, 0, origin, direction);

        optixTrace(
            mesh_params.handle,
            origin,
            direction,
            0.0f,
            1e5f,
            1.0f,
            OptixVisibilityMask(255),
            unsigned(OPTIX_RAY_FLAG_NONE),
            unsigned(0),
            unsigned(1),
            unsigned(0),
            Payload_As_Params(payload));
        patch.uv0 = payload.uv;

        calculateRayParameters(
            launch_index, launch_dimensions, 1, 0, origin, direction);
        optixTrace(
            mesh_params.handle,
            origin,
            direction,
            0.0f,
            1e5f,
            1.0f,
            OptixVisibilityMask(255),
            unsigned(OPTIX_RAY_FLAG_NONE),
            unsigned(0),
            unsigned(1),
            unsigned(0),
            Payload_As_Params(payload));
        patch.uv1 = payload.uv;

        calculateRayParameters(
            launch_index, launch_dimensions, 1, 1, origin, direction);

        optixTrace(
            mesh_params.handle,
            origin,
            direction,
            0.0f,
            1e5f,
            1.0f,
            OptixVisibilityMask(255),
            unsigned(OPTIX_RAY_FLAG_NONE),
            unsigned(0),
            unsigned(1),
            unsigned(0),
            Payload_As_Params(payload));

        patch.uv2 = payload.uv;
        calculateRayParameters(
            launch_index, launch_dimensions, 0, 1, origin, direction);

        optixTrace(
            mesh_params.handle,
            origin,
            direction,
            0.0f,
            1e5f,
            1.0f,
            OptixVisibilityMask(255),
            unsigned(OPTIX_RAY_FLAG_NONE),
            unsigned(0),
            unsigned(1),
            unsigned(0),
            Payload_As_Params(payload));
        patch.uv3 = payload.uv;

        auto id = mesh_params.append_buffer->Push(patch);
        mesh_params.corners[id].v0 = payload.corner0;
        mesh_params.corners[id].v1 = payload.corner1;
        mesh_params.corners[id].v2 = payload.corner2;
        mesh_params.pixel_targets[id] =
            make_int2(launch_index.x, launch_index.y);
    }
}

CHS(mesh)
{
    Payload payload;
    auto primitiveid = optixGetPrimitiveIndex();
    uint3 indices = reinterpret_cast<uint3*>(mesh_params.indices)[primitiveid];
    payload.corner0 =
        reinterpret_cast<float3*>(mesh_params.vertices)[indices.x];
    payload.corner1 =
        reinterpret_cast<float3*>(mesh_params.vertices)[indices.y];
    payload.corner2 =
        reinterpret_cast<float3*>(mesh_params.vertices)[indices.z];

    payload.uv = optixGetTriangleBarycentrics();

    payload.hit = true;
    payload.set_self();
}

MISS(mesh)
{
}

AHS(mesh)
{
}
