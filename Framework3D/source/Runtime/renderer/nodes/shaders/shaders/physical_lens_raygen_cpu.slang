
import lens_shader_cpu;
import utils.ray;
#include "utils/CameraParameters.h"
import Utils.Math.MathHelpers;
#include "utils/random.slangh"
#include "utils/view_cb.h"

ConstantBuffer<LensSystemData> lens_system_data;
RWStructuredBuffer<RayInfo> rays;

[shader("compute")]
[numthreads(128, 1, 1)]
void ray_trace_main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint id = dispatchThreadID.x;

    uint ray_count;
    uint stride;

    rays.GetDimensions(ray_count, stride);

    if (id >= ray_count) {
        return;
    }

    var ray = rays[id];
    var traced = ray_trace(ray, lens_system_data, id);
    rays[id] = traced;
}
