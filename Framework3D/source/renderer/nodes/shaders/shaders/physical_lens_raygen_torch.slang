
import lens_shader;
import utils.ray;
#include "utils/CameraParameters.h"
import Utils.Math.MathHelpers;
#include "utils/random.slangh"
#include "utils/view_cb.h"

[Differentiable]
void fill_ray(inout DiffTensorView tensor, uint2 id, RayInfo ray)
{
    tensor[uint3(id.x, id.y, 0)] = ray.Origin.x;
    tensor[uint3(id.x, id.y, 1)] = ray.Origin.y;
    tensor[uint3(id.x, id.y, 2)] = ray.Origin.z;
    tensor[uint3(id.x, id.y, 3)] = ray.Direction.x;
    tensor[uint3(id.x, id.y, 4)] = ray.Direction.y;
    tensor[uint3(id.x, id.y, 5)] = ray.Direction.z;
    tensor[uint3(id.x, id.y, 6)] = ray.TMin;
    tensor[uint3(id.x, id.y, 7)] = ray.TMax;
    tensor[uint3(id.x, id.y, 8)] = ray.throughput.toFloat3().x;
    tensor[uint3(id.x, id.y, 9)] = ray.throughput.toFloat3().y;
    tensor[uint3(id.x, id.y, 10)] = ray.throughput.toFloat3().z;
}

[AutoPyBindCUDA]
[CUDAKernel]
[Differentiable]
void computeMain(
    TensorView<int> random_seeds,
    DiffTensorView lens_system_data_tensor,
    DiffTensorView rays,
    TensorView<int> pixel_targets)
{
    uint3 dispatchThreadID = cudaThreadIdx() + cudaBlockIdx() * cudaBlockDim();
    uint2 LaunchIndex = dispatchThreadID.xy;
    uint2 LaunchDimensions;
    LaunchDimensions.x = random_seeds.size(0);
    LaunchDimensions.y = random_seeds.size(1);

    LensSystemData lens_system_data =
        get_lens_data_from_torch_tensor(lens_system_data_tensor);

    if (all(LaunchIndex.xy < LaunchDimensions.xy)) {
        uint index = LaunchIndex.x + LaunchIndex.y * LaunchDimensions.x;
        float3 weight = 1.0;
        uint seed = random_seeds[LaunchIndex];

        RayInfo ray;
        ray = raygen(LaunchIndex, weight, seed, lens_system_data);
        ray.throughput = { weight };
        ray.Direction.z *= -1;
        ray.Origin /= 1000.f;
        ray.TMin = 0;
        ray.TMax = 1000;

        fill_ray(rays, LaunchIndex, ray);

        pixel_targets[LaunchIndex.x * LaunchDimensions.y + LaunchIndex.y] =
            LaunchIndex.x * LaunchDimensions.y + LaunchIndex.y;
        random_seeds[LaunchIndex] = seed;
    }
}
