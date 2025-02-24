float2 mx_transform_uv(float2 uv, float2 uv_scale, float2 uv_offset)
{
    uv = uv * uv_scale + uv_offset;
    return float2(uv.x, 1.0 - uv.y);
}
