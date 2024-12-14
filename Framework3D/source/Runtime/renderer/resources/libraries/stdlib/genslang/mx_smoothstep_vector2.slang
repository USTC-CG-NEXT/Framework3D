#include "mx_smoothstep_float.slang"

void mx_smoothstep_vector2(float2 val, float2 low, float2 high, out float2 result)
{
    mx_smoothstep_float(val.x, low.x, high.x, result.x);
    mx_smoothstep_float(val.y, low.y, high.y, result.y);
}
