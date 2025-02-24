#include "mx_burn_float.slang"

void mx_burn_color4(float4 fg, float4 bg, float mixval, out float4 result)
{
    mx_burn_float(fg.x, bg.x, mixval, result.x);
    mx_burn_float(fg.y, bg.y, mixval, result.y);
    mx_burn_float(fg.z, bg.z, mixval, result.z);
    mx_burn_float(fg.w, bg.w, mixval, result.w);
}
