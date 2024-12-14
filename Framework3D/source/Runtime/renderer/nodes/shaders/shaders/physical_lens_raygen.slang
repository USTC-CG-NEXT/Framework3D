
import lens_shader;
import utils.ray;
#include "utils/CameraParameters.h"
import Utils.Math.MathHelpers;
#include "utils/random.slangh"
#include "utils/view_cb.h"

RWStructuredBuffer<RayInfo> rays;
RWTexture2D<uint> random_seeds;

ConstantBuffer<LensSystemData> lens_system_data;

RWStructuredBuffer<uint2> pixel_targets;
ConstantBuffer<PlanarViewConstants> viewConstant;

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint2 LaunchIndex = dispatchThreadID.xy;
    uint2 LaunchDimensions = viewConstant.resolution;

    if (all(LaunchIndex.xy < LaunchDimensions.xy)) {
        uint index = LaunchIndex.x * LaunchDimensions.y + LaunchIndex.y;

        uint seed = random_seeds[LaunchIndex];

        float x = random_float(seed);
        float y = random_float(seed);

        float pixel_target_x = x * viewConstant.resolution.x;
        float pixel_target_y = y * viewConstant.resolution.y;
        uint2 pixel_target = { floor(pixel_target_x), floor(pixel_target_y) };
        float2 pixel_target_f = { pixel_target_x, pixel_target_y };

        float2 seed2 = random_float2(seed);
        RayInfo ray;
        ray = raygen(detach(pixel_target_f), seed2, lens_system_data);
        ray.Direction.z *= -1;
        ray.Origin /= 1000.f;
        ray.TMin = 0;
        ray.TMax = 1000;

        ray = transformRay(ray, viewConstant.matViewToWorld);
        rays[index] = ray;
        pixel_targets[index] = pixel_target;
        random_seeds[LaunchIndex] = seed;
    }
}
