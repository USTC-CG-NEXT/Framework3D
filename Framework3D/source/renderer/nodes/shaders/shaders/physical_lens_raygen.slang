
import lens_shader;
#include "utils/CameraParameters.h"
import Utils.Math.MathHelpers;
#include "utils/random.slangh"
#include "utils/view_cb.h"

RWStructuredBuffer<RayInfo> rays;
RWTexture2D<uint> random_seeds;

RWStructuredBuffer<uint2> pixel_targets;
ConstantBuffer<PlanarViewConstants> viewConstant;

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint2 LaunchIndex = dispatchThreadID.xy;
    uint2 LaunchDimensions = viewConstant.resolution;

    if (all(LaunchIndex.xy < LaunchDimensions.xy)) {
        uint index = LaunchIndex.x + LaunchIndex.y * LaunchDimensions.x;
        float weight = 1.0;
        uint seed = random_seeds[LaunchIndex];

        RayInfo ray;
        ray = raygen(LaunchIndex, weight, seed);
        ray.TMin = 0;
        ray.TMax = 1000;

        ray = transformRay(ray, viewConstant.matViewToWorld);
        
        rays[LaunchIndex.x * LaunchDimensions.y + LaunchIndex.y] = ray;

        pixel_targets[LaunchIndex.x * LaunchDimensions.y + LaunchIndex.y] =
            LaunchIndex;
        random_seeds[LaunchIndex] = seed;
    }
}
