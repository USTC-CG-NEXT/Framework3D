#include "lib/mx_noise.slang"

void mx_noise3d_float(float amplitude, float pivot, float3 position, out float result)
{
    float value = mx_perlin_noise_float(position);
    result = value * amplitude + pivot;
}
