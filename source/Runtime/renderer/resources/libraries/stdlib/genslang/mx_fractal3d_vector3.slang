#include "lib/mx_noise.slang"

void mx_fractal3d_vector3(float3 amplitude, int octaves, float lacunarity, float diminish, float3 position, out float3 result)
{
    float3 value = mx_fractal_noise_vec3(position, octaves, lacunarity, diminish);
    result = value * amplitude;
}
