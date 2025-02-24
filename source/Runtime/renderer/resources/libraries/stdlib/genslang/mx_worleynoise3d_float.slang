#include "lib/mx_noise.slang"

void mx_worleynoise3d_float(float3 position, float jitter, out float result)
{
    result = mx_worley_noise_float(position, jitter, 0);
}
