#include "mx_aastep.slang"

void mx_splitlr_vector3(float3 valuel, float3 valuer, float center, float2 texcoord, out float3 result)
{
    result = lerp(valuel, valuer, mx_aastep(center, texcoord.x));
}
