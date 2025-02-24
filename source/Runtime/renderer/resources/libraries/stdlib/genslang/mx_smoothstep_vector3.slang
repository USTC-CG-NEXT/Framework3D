#include "mx_smoothstep_float.slang"

void mx_smoothstep_vector3(float3 val, float3 low, float3 high, out float3 result)
{
    mx_smoothstep_float(val.x, low.x, high.x, result.x);
    mx_smoothstep_float(val.y, low.y, high.y, result.y);
    mx_smoothstep_float(val.z, low.z, high.z, result.z);
}
