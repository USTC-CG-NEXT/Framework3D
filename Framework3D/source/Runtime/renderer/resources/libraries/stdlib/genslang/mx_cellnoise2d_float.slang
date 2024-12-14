#include "lib/mx_noise.slang"

void mx_cellnoise2d_float(float2 texcoord, out float result)
{
    result = mx_cell_noise_float(texcoord);
}
