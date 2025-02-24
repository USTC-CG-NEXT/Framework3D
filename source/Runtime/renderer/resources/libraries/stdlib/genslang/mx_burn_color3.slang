#include "mx_burn_float.slang"

void mx_burn_color3(float3 fg, float3 bg, float mixval, out float3 result)
{
    mx_burn_float(fg.x, bg.x, mixval, result.x);
    mx_burn_float(fg.y, bg.y, mixval, result.y);
    mx_burn_float(fg.z, bg.z, mixval, result.z);
}
