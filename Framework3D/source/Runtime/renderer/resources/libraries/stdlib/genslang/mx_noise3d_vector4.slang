#include "lib/mx_noise.slang"

void mx_noise3d_vector4(float4 amplitude, float pivot, float3 position, out float4 result)
{
    float3 xyz = mx_perlin_noise_vec3(position);
    float w = mx_perlin_noise_float(position + float3(19, 73, 29));
    result = float4(xyz, w) * amplitude + pivot;
}
