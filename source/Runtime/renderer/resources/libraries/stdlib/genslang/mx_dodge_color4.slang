#include "mx_dodge_float.slang"

void mx_dodge_color4(float4 fg , float4 bg , float mixval, out float4 result)
{
    mx_dodge_float(fg.x, bg.x, mixval, result.x);
    mx_dodge_float(fg.y, bg.y, mixval, result.y);
    mx_dodge_float(fg.z, bg.z, mixval, result.z);
    mx_dodge_float(fg.w, bg.w, mixval, result.w);
}
