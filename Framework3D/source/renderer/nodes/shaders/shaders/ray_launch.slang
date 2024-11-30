/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

// ---[ Structures ]---
#include "matrix_op.h"
#include "utils/HitObject.h"
import utils.ray;
#include "utils/view_cb.h"

RaytracingAccelerationStructure SceneBVH : register(t0);
StructuredBuffer<uint2> inPixelTarget : register(t1);
StructuredBuffer<RayInfo> rays : register(t2);
RWStructuredBuffer<HitObjectInfo> hitObjectInfo : register(u0);
RWStructuredBuffer<uint2> PixelTarget : register(u1);
RWStructuredBuffer<uint> hit_counter_buffer : register(u2);
RWStructuredBuffer<uint> miss_counter_buffer : register(u3);

struct HitInfo {
    float2 barycentric;
    float3 position;
    float4 ShadedColorAndHitT : SHADED_COLOR_AND_HIT_T;
};

struct Attributes {
    float2 uv;
};

// ---[ Ray Generation Shader ]---

[shader("raygeneration")]
void RayGen()
{
    uint LaunchIndex = DispatchRaysIndex().x;
    uint LaunchDimensions = DispatchRaysDimensions().x;

    RayInfo ray = rays[LaunchIndex];

    HitInfo payload;
    payload.ShadedColorAndHitT = float4(0, 0, 0, 0);

    TraceRay(
        SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, get_ray_desc(ray), payload);
}

// ---[ Closest Hit Shader ]---

[shader("closesthit")]
void ClosestHit(
    inout HitInfo payload: SV_RayPayload,
    Attributes attrib: SV_IntersectionAttributes)
{
    payload.barycentric = attrib.uv;
    payload.position = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    // Push hit information into hitObjectInfo buffer
    uint stride_count = 0;
    uint stride_size = 0;
    hitObjectInfo.GetDimensions(stride_count, stride_size);

    uint index;
    InterlockedAdd(hit_counter_buffer[0], 1u, index);

    hitObjectInfo[index].InstanceIndex = InstanceIndex();
    hitObjectInfo[index].GeometryIndex = GeometryIndex();
    hitObjectInfo[index].PrimitiveIndex = PrimitiveIndex();
    hitObjectInfo[index].HitKind = HitKind();

    hitObjectInfo[index].RayContributionToHitGroupIndex = 0;
    hitObjectInfo[index].MultiplierForGeometryContributionToHitGroupIndex = 0;
    hitObjectInfo[index].rays = rays[DispatchRaysIndex().x];

    hitObjectInfo[index].attributes = payload.barycentric;
    PixelTarget[index] = inPixelTarget[DispatchRaysIndex().x];
}

// ---[ Miss Shader ]---

[shader("miss")]
void Miss(inout HitInfo payload: SV_RayPayload)
{
    uint stride_count = 0;
    uint stride_size = 0;
    hitObjectInfo.GetDimensions(stride_count, stride_size);

    uint index;
    InterlockedAdd(miss_counter_buffer[0], 1u, index);
}
