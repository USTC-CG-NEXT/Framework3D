void mx_luminance_color4(float4 _in, float3 lumacoeffs, out float4 result)
{
    result = float4(float3(dot(_in.rgb, lumacoeffs)), _in.a);
}
