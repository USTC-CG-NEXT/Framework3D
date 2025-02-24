#include "lib/mx_noise.slang"

void mx_noise2d_vector2(float2 amplitude, float pivot, float2 texcoord, out float2 result)
{
    float3 value = mx_perlin_noise_vec3(texcoord);
    result = value.xy * amplitude + pivot;
}
