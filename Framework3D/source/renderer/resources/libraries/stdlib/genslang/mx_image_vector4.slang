#include "lib/$fileTransformUv"

void mx_image_vector4(Sampler2D tex_sampler, int layer, float4 defaultval, float2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, float2 uv_scale, float2 uv_offset, out float4 result)
{
    float2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    result = texture(tex_sampler, uv);
}
