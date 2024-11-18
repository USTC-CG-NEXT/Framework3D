#include "mx_aastep.slang"

void mx_splittb_vector3(float3 valuet, float3 valueb, float center, float2 texcoord, out float3 result)
{
    result = lerp(valuet, valueb, mx_aastep(center, texcoord.y));
}
