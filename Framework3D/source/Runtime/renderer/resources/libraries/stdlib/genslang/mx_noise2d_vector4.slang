#include "lib/mx_noise.slang"

void mx_noise2d_vector4(float4 amplitude, float pivot, float2 texcoord, out float4 result)
{
    float3 xyz = mx_perlin_noise_vec3(texcoord);
    float w = mx_perlin_noise_float(texcoord + float2(19, 73));
    result = float4(xyz, w) * amplitude + pivot;
}
