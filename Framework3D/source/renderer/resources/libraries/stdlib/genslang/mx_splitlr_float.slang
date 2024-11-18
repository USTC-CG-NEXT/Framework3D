#include "mx_aastep.slang"

void mx_splitlr_float(float valuel, float valuer, float center, float2 texcoord, out float result)
{
    result = lerp(valuel, valuer, mx_aastep(center, texcoord.x));
}
