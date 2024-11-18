#include "lib/$fileTransformUv"

void mx_image_vector2(Sampler2D tex_sampler, int layer, float2 defaultval, float2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, float2 uv_scale, float2 uv_offset, out float2 result)
{
    float2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    result = texture(tex_sampler, uv).rg;
}
