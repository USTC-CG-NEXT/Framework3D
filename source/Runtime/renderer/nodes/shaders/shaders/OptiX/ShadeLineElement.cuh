#pragma once

#include "line_special_shading.cuh"

__device__ inline float signed_area(const LineSegment& line, float2 point)
{
    // The direction is expected to be normalized
    return cross(point - line.position, line.direction);
}

__device__ inline float ShadeLineElement(
    const LineSegment& line,
    const Patch& patch,
    const GlintsTracingParams& glints_tracing_params)
{
    float3 camera_pos_uv = patch.camera_pos_uv;
    float3 light_pos_uv = patch.light_pos_uv;

    auto p0 = patch.uv0;
    auto p1 = patch.uv1;
    auto p2 = patch.uv2;
    auto p3 = patch.uv3;

    auto center = (p0 + p1 + p2 + p3) / 4.0;

    auto p = make_float3(center, 0.0);

    float3 camera_dir = normalize(camera_pos_uv - p);

    float3 light_dir = normalize(light_pos_uv - p);

    float3 local_cam_dir = make_float3(
        cross(make_float2(camera_dir), line.direction),
        dot(make_float2(camera_dir), line.direction),
        camera_dir.z);

    float3 local_light_dir = make_float3(
        cross(make_float2(light_dir), line.direction),
        dot(make_float2(light_dir), line.direction),
        light_dir.z);

    float3 half_vec = normalize((local_cam_dir + local_light_dir) / 2);

    //float xPara = +
    //    cross(make_float2(light_dir), line.direction);
    //float yPara =
    //    dot(make_float2(camera_dir), line.direction) + dot(make_float2(light_dir), line.direction);
    //float zPara = camera_dir.z + light_dir.z;



    auto a0 = signed_area(line, p0);
    auto a1 = signed_area(line, p1);
    auto a2 = signed_area(line, p2);
    auto a3 = signed_area(line, p3);

    auto minimum = min(min(min(a0, a1), a2), a3);
    auto maximum = max(max(max(a0, a1), a2), a3);

    float width = 1.0 / glints_tracing_params.width;

    float cut = 0.4;

    if (minimum * maximum > 0)
    {
        if (abs(minimum) > cut * width && abs(maximum) > cut * width)
        {
            return 0.0;
        }
    }

    auto temp =
        lineShade(
            max(minimum, -cut * width),
            min(maximum, cut * width),
            0.01,
            half_vec.x,
            half_vec.z,
            width) /
        length(light_pos_uv - p) / length(light_pos_uv - p);

    return temp * glints_tracing_params.exposure * 100;
}
