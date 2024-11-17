#include "lib/mx_noise.slang"

void mx_worleynoise2d_float(float2 texcoord, float jitter, out float result)
{
    result = mx_worley_noise_float(texcoord, jitter, 0);
}
