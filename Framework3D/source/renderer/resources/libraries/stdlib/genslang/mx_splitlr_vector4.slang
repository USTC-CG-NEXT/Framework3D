#include "mx_aastep.slang"

void mx_splitlr_vector4(float4 valuel, float4 valuer, float center, float2 texcoord, out float4 result)
{
    result = lerp(valuel, valuer, mx_aastep(center, texcoord.x));
}
