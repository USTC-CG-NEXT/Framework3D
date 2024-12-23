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
    float4x4 worldToUV;
    unsigned hit;

    void set_self()
    {
        optixSetPayload_0(__float_as_uint(uv.x));
        optixSetPayload_1(__float_as_uint(uv.y));
        optixSetPayload_2(__float_as_uint(worldToUV.m[0][0]));
        optixSetPayload_3(__float_as_uint(worldToUV.m[0][1]));
        optixSetPayload_4(__float_as_uint(worldToUV.m[0][2]));
        optixSetPayload_5(__float_as_uint(worldToUV.m[0][3]));
        optixSetPayload_6(__float_as_uint(worldToUV.m[1][0]));
        optixSetPayload_7(__float_as_uint(worldToUV.m[1][1]));
        optixSetPayload_8(__float_as_uint(worldToUV.m[1][2]));
        optixSetPayload_9(__float_as_uint(worldToUV.m[1][3]));
        optixSetPayload_10(__float_as_uint(worldToUV.m[2][0]));
        optixSetPayload_11(__float_as_uint(worldToUV.m[2][1]));
        optixSetPayload_12(__float_as_uint(worldToUV.m[2][2]));
        optixSetPayload_13(__float_as_uint(worldToUV.m[2][3]));
        optixSetPayload_14(__float_as_uint(worldToUV.m[3][0]));
        optixSetPayload_15(__float_as_uint(worldToUV.m[3][1]));
        optixSetPayload_16(__float_as_uint(worldToUV.m[3][2]));
        optixSetPayload_17(__float_as_uint(worldToUV.m[3][3]));
        optixSetPayload_18(hit);
    }
};

#define Payload_As_Params(payload_name)                                  \
    reinterpret_cast<unsigned int&>(payload_name.uv.x),                  \
        reinterpret_cast<unsigned int&>(payload_name.uv.y),              \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[0][0]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[0][1]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[0][2]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[0][3]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[1][0]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[1][1]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[1][2]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[1][3]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[2][0]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[2][1]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[2][2]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[2][3]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[3][0]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[3][1]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[3][2]), \
        reinterpret_cast<unsigned int&>(payload_name.worldToUV.m[3][3]), \
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

    auto clipToView = mesh_params.viewToClip.get_inverse();
    auto viewToWorld = mesh_params.worldToView.get_inverse();

    float4 view_pos = clipToView * clip_pos;
    view_pos /= view_pos.w;

    auto view_space_direction = (make_float3(view_pos) - make_float3(0, 0, 0));

    origin = make_float3(viewToWorld * make_float4(0, 0, 0, 1));
    direction = normalize(
        make_float3(viewToWorld * make_float4(view_space_direction, 0)));
}

__device__ void traceRayAndSetPayload(
    const uint3& launch_index,
    const uint3& launch_dimensions,
    float bias_x,
    float bias_y,
    float3& origin,
    float3& direction,
    Payload& payload)
{
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
}

RGS(mesh)
{
    uint3 launch_index = optixGetLaunchIndex();
    uint3 launch_dimensions = optixGetLaunchDimensions();

    float3 origin;
    float3 direction;

    Payload payload;
    payload.hit = false;

    traceRayAndSetPayload(
        launch_index,
        launch_dimensions,
        0.5f,
        0.5f,
        origin,
        direction,
        payload);

    if (payload.hit) {
        Patch patch;

        traceRayAndSetPayload(
            launch_index, launch_dimensions, 0, 0, origin, direction, payload);
        patch.uv0 = payload.uv;

        traceRayAndSetPayload(
            launch_index, launch_dimensions, 1, 0, origin, direction, payload);
        patch.uv1 = payload.uv;

        traceRayAndSetPayload(
            launch_index, launch_dimensions, 1, 1, origin, direction, payload);
        patch.uv2 = payload.uv;

        traceRayAndSetPayload(
            launch_index, launch_dimensions, 0, 1, origin, direction, payload);
        patch.uv3 = payload.uv;

        auto id = mesh_params.append_buffer->Push(patch);
        mesh_params.worldToUV[id] = payload.worldToUV;
        mesh_params.pixel_targets[id] =
            make_int2(launch_index.x, launch_index.y);
    }
}

__device__ float2 operator*(const float2& a, const float b)
{
    return make_float2(a.x * b, a.y * b);
}

__device__ float3 cross(const float3& a, const float3& b)
{
    return make_float3(
        a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
__device__ float3 operator-(const float3& a)
{
    return make_float3(-a.x, -a.y, -a.z);
}
struct Vertex {
    float pos_x, pos_y, pos_z;
    float u, v;
};

CHS(mesh)
{
    Payload payload;
    auto primitiveid = optixGetPrimitiveIndex();
    uint3 indices = reinterpret_cast<uint3*>(mesh_params.indices)[primitiveid];

    auto vertex0 = reinterpret_cast<Vertex*>(mesh_params.vertices)[indices.x];
    auto vertex1 = reinterpret_cast<Vertex*>(mesh_params.vertices)[indices.y];
    auto vertex2 = reinterpret_cast<Vertex*>(mesh_params.vertices)[indices.z];

    float2 uv0 = make_float2(vertex0.u, vertex0.v);
    float2 uv1 = make_float2(vertex1.u, vertex1.v);
    float2 uv2 = make_float2(vertex2.u, vertex2.v);

    float3 pos0 = make_float3(vertex0.pos_x, vertex0.pos_y, vertex0.pos_z);
    float3 pos1 = make_float3(vertex1.pos_x, vertex1.pos_y, vertex1.pos_z);
    float3 pos2 = make_float3(vertex2.pos_x, vertex2.pos_y, vertex2.pos_z);

    float3 normal = normalize(cross(pos1 - pos0, pos2 - pos0));

    float4x4 target(
        make_float4(uv0.x, uv0.y, 0.f, 1.f),
        make_float4(uv1.x, uv1.y, 0.f, 1.f),
        make_float4(uv2.x, uv2.y, 0.f, 1.f),
        make_float4(0, 0, 1, 0));
    float4x4 points(
        make_float4(pos0.x, pos0.y, pos0.z, 1),
        make_float4(pos1.x, pos1.y, pos1.z, 1),
        make_float4(pos2.x, pos2.y, pos2.z, 1),
        make_float4(normal, 0));

    payload.worldToUV = target * points.get_inverse();

    auto barycentric = optixGetTriangleBarycentrics();

    payload.uv = uv0 * (1.0f - barycentric.x - barycentric.y) +
                 uv1 * barycentric.x + uv2 * barycentric.y;

    payload.hit = true;
    payload.set_self();
}

MISS(mesh)
{
}

AHS(mesh)
{
}
