#include "lib/mx_hsv.slang"

void mx_hsvtorgb_color4(float4 _in, out float4 result)
{
    result = float4(mx_hsvtorgb(_in.rgb), 1.0);
}
