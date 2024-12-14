#include "lib/mx_noise.slang"

void mx_noise2d_float(float amplitude, float pivot, float2 texcoord, out float result)
{
    float value = mx_perlin_noise_float(texcoord);
    result = value * amplitude + pivot;
}
