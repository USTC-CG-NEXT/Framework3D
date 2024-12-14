
import lens_shader;
import utils.ray;
#include "utils/CameraParameters.h"
import Utils.Math.MathHelpers;
#include "utils/random.slangh"
#include "utils/view_cb.h"

[Differentiable]
void fill_ray(inout DiffTensorView tensor, uint id, RayInfo ray)
{
    tensor[uint2(id.x, 0)] = ray.Origin.x;
    tensor[uint2(id.x, 1)] = ray.Origin.y;
    tensor[uint2(id.x, 2)] = ray.Origin.z;
    tensor[uint2(id.x, 3)] = ray.Direction.x;
    tensor[uint2(id.x, 4)] = ray.Direction.y;
    tensor[uint2(id.x, 5)] = ray.Direction.z;
    tensor[uint2(id.x, 6)] = ray.TMin;
    tensor[uint2(id.x, 7)] = ray.TMax;
    tensor[uint2(id.x, 8)] = ray.throughput.toFloat3().x;
    tensor[uint2(id.x, 9)] = ray.throughput.toFloat3().y;
    tensor[uint2(id.x, 10)] = ray.throughput.toFloat3().z;
}

[AutoPyBindCUDA]
[CUDAKernel]
[Differentiable]
void computeMain(
    TensorView<float2> sample1,
    TensorView<float2> sample2,
    DiffTensorView lens_system_data_tensor,
    DiffTensorView rays)
{
    uint3 dispatchThreadID = cudaThreadIdx() + cudaBlockIdx() * cudaBlockDim();
    uint sample_id = dispatchThreadID.x;
    uint launch_count = sample1.size(0);

    LensSystemData lens_system_data =
        get_lens_data_from_torch_tensor(lens_system_data_tensor);

    if (sample_id < launch_count) {
        float3 weight = 1.0;
        float2 pos_sample = no_diff sample1[sample_id];
        float2 aperture_sample = no_diff sample2[sample_id];

        float pixel_target_x =
            pos_sample.x * lens_system_data.film_resolution.x;
        float pixel_target_y =
            pos_sample.y * lens_system_data.film_resolution.y;
        float2 pixel_target_f = { pixel_target_x, pixel_target_y };

        RayInfo ray;
        ray = raygen(pixel_target_f, weight, aperture_sample, lens_system_data);
        ray.Direction.z *= -1;
        ray.Origin /= 1000.f;
        ray.TMin = 0;
        ray.TMax = 1000;
        ray.throughput = { weight };

        fill_ray(rays, sample_id, ray);
    }
}
