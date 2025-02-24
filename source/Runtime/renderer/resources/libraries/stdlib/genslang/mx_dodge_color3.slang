#include "mx_dodge_float.slang"

void mx_dodge_color3(float3 fg, float3 bg, float mixval, out float3 result)
{
    mx_dodge_float(fg.x, bg.x, mixval, result.x);
    mx_dodge_float(fg.y, bg.y, mixval, result.y);
    mx_dodge_float(fg.z, bg.z, mixval, result.z);
}
