#include "lib/mx_noise.slang"

void mx_noise3d_vector2(float2 amplitude, float pivot, float3 position, out float2 result)
{
    float3 value = mx_perlin_noise_vec3(position);
    result = value.xy * amplitude + pivot;
}
