void mx_luminance_color3(float3 _in, float3 lumacoeffs, out float3 result)
{
    result = float3(dot(_in, lumacoeffs));
}
