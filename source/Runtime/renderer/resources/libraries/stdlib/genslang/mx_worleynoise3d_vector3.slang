#include "lib/mx_noise.slang"

void mx_worleynoise3d_vector3(float3 position, float jitter, out float3 result)
{
    result = mx_worley_noise_vec3(position, jitter, 0);
}
