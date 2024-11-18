#include "mx_aastep.slang"

void mx_splittb_float(float valuet, float valueb, float center, float2 texcoord, out float result)
{
    result = lerp(valuet, valueb, mx_aastep(center, texcoord.y));
}
