#include "lib/mx_hsv.slang"

void mx_rgbtohsv_color3(float3 _in, out float3 result)
{
    result = mx_rgbtohsv(_in);
}
