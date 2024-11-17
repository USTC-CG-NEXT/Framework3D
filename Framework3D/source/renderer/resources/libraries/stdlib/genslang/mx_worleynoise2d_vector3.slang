#include "lib/mx_noise.slang"

void mx_worleynoise2d_vector3(float2 texcoord, float jitter, out float3 result)
{
    result = mx_worley_noise_vec3(texcoord, jitter, 0);
}
