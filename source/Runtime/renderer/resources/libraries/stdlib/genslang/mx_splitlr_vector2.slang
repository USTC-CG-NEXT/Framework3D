#include "mx_aastep.slang"

void mx_splitlr_vector2(float2 valuel, float2 valuer, float center, float2 texcoord, out float2 result)
{
    result = lerp(valuel, valuer, mx_aastep(center, texcoord.x));
}
