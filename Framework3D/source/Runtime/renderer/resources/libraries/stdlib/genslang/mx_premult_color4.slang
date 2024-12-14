void mx_premult_color4(float4 _in, out float4 result)
{
    result = float4(_in.rgb * _in.a, _in.a);
}
