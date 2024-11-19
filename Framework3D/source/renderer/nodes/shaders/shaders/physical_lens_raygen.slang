
import lens_shader;
#include "utils/view_cb.h"

RWStructuredBuffer<RayInfo> rays;
RWTexture2D<uint> random_seeds;

RWStructuredBuffer<uint2> pixel_targets;
ConstantBuffer<uint2> size;
ConstantBuffer<PlanarViewConstants> viewConstant;

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint2 pixel_id = dispatchThreadID.xy;

    if (pixel_id.x >= size.x || pixel_id.y >= size.y) {
        return;
    }

    uint index = pixel_id.x + pixel_id.y * size.x;

    float weight = 1.0;

    uint seed = random_seeds[pixel_id];
    //seed = index;
    RayInfo ray;
    var temp = raygen(pixel_id, weight, seed);
    ray.Origin = weight * temp.Origin * 1;
    ray.Direction = weight * temp.Direction;
    ray.TMin = 0;
    ray.TMax = 1000;

    ray = transformRay(ray, viewConstant.matViewToWorld);

    rays[index] = ray;
    random_seeds[pixel_id] = seed;
    pixel_targets[index] = uint2(pixel_id);
}
