#include "mx_aastep.slang"

void mx_splittb_vector2(float2 valuet, float2 valueb, float center, float2 texcoord, out float2 result)
{
    result = lerp(valuet, valueb, mx_aastep(center, texcoord.y));
}
