#include "lib/mx_noise.slang"

void mx_worleynoise3d_vector2(float3 position, float jitter, out float2 result)
{
    result = mx_worley_noise_vec2(position, jitter, 0);
}
