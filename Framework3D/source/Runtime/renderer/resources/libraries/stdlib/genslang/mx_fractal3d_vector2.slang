#include "lib/mx_noise.slang"

void mx_fractal3d_vector2(float2 amplitude, int octaves, float lacunarity, float diminish, float3 position, out float2 result)
{
    float2 value = mx_fractal_noise_vec2(position, octaves, lacunarity, diminish);
    result = value * amplitude;
}
