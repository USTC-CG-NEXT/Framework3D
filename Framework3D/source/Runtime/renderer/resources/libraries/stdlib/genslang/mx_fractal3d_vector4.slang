#include "lib/mx_noise.slang"

void mx_fractal3d_vector4(float4 amplitude, int octaves, float lacunarity, float diminish, float3 position, out float4 result)
{
    float4 value = mx_fractal_noise_vec4(position, octaves, lacunarity, diminish);
    result = value * amplitude;
}
