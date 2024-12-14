#include "lib/mx_noise.slang"

void mx_noise2d_vector3(float3 amplitude, float pivot, float2 texcoord, out float3 result)
{
    float3 value = mx_perlin_noise_vec3(texcoord);
    result = value * amplitude + pivot;
}
