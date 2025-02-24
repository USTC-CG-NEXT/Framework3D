#include "lib/mx_noise.slang"

void mx_worleynoise2d_vector2(float2 texcoord, float jitter, out float2 result)
{
    result = mx_worley_noise_vec2(texcoord, jitter, 0);
}
