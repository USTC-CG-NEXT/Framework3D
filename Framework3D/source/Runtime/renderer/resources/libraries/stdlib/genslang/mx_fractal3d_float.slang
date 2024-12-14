#include "lib/mx_noise.slang"

void mx_fractal3d_float(float amplitude, int octaves, float lacunarity, float diminish, float3 position, out float result)
{
    float value = mx_fractal_noise_float(position, octaves, lacunarity, diminish);
    result = value * amplitude;
}
