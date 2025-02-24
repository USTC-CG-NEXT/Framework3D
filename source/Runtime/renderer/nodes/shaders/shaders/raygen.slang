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
#include "utils/CameraParameters.h"
#include "utils/random.slangh"
import utils.ray;
import utils.Spectrum;
#include "utils/view_cb.h"

import Utils.Math.MathHelpers;

RWStructuredBuffer<RayInfo> rays : register(u0);
RWTexture2D<uint> random_seeds : register(u1);
RWStructuredBuffer<uint2> pixel_targets : register(u2);

// ---[ Constant Buffers ]---
ConstantBuffer<PlanarViewConstants> viewConstant : register(b0);

ConstantBuffer<CameraParameters> cameraParams : register(b1);

RayInfo setupPrimaryRay(
    uint2 pixelPosition,
    PlanarViewConstants view,
    inout uint seed,
    CameraParameters cameraParams)
{
    float2 pixelPositionF = float2(pixelPosition);
    pixelPositionF.x += random_float(seed);
    pixelPositionF.y += random_float(seed);

    float2 uv = pixelPositionF * view.viewportSizeInv;
    float4 clipPos = float4(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1, 1.0, 1);

    float4 viewPos = mul(clipPos, view.matClipToView);
    viewPos.xyz /= viewPos.w;

    RayInfo ray;
    ray.Origin = float3(0, 0, 0);

    // Calculate cameraRight and cameraUp
    float3 cameraRight = (float3(1, 0, 0));
    float3 cameraUp = (float3(0, 1, 0));

    // Apply depth of field effect
    float lensRadius = cameraParams.aperture * 0.5;
    float2 seed2 = float2(random_float(seed), random_float(seed));
    float2 randomDisk = lensRadius * sample_disk(seed2);
    float3 offset = cameraRight * randomDisk.x + cameraUp * randomDisk.y;
    float3 focusPoint = ray.Origin + cameraParams.focusDistance *
                                         normalize(viewPos.xyz - ray.Origin);

    ray.Origin += offset;
    ray.Direction = normalize(focusPoint - ray.Origin);
    ray.TMin = 0;
    ray.TMax = 10000;
    return ray;
}

[numthreads(32, 32, 1)]
void main(uint2 DTid: SV_DispatchThreadID)
{
    uint2 LaunchIndex = DTid.xy;
    uint2 LaunchDimensions = viewConstant.resolution;

    if (all(LaunchIndex.xy < LaunchDimensions.xy)) {
        uint seed = random_seeds[LaunchIndex];

#if SCATTER_RAYS
        float x = random_float(seed);
        float y = random_float(seed);

        uint pixel_target_x = floor(x * viewConstant.resolution.x);
        uint pixel_target_y = floor(y * viewConstant.resolution.y);
        uint2 pixel_target = { pixel_target_x, pixel_target_y };
#else
        uint2 pixel_target = LaunchIndex;
#endif
        // Setup the ray
        CameraParameters c;
        c.focusDistance = cameraParams.focusDistance;
        c.aperture = cameraParams.aperture;
        RayInfo ray = setupPrimaryRay(pixel_target, viewConstant, seed, c);
        ray.throughput = RGBSpectrum(float3(1.0f));

        // Transform the ray to world space
        ray = transformRay(ray, viewConstant.matViewToWorld);
        rays[LaunchIndex.x * LaunchDimensions.y + LaunchIndex.y] = ray;
        pixel_targets[LaunchIndex.x * LaunchDimensions.y + LaunchIndex.y] =
            pixel_target;
        random_seeds[LaunchIndex] = seed;
    }
}
