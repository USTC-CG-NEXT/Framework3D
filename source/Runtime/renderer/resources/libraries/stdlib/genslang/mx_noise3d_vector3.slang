#include "lib/mx_noise.slang"

void mx_noise3d_vector3(float3 amplitude, float pivot, float3 position, out float3 result)
{
    float3 value = mx_perlin_noise_vec3(position);
    result = value * amplitude + pivot;
}
