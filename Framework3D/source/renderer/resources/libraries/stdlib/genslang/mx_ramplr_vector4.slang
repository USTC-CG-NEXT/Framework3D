void mx_ramplr_vector4(float4 valuel, float4 valuer, float2 texcoord, out float4 result)
{
    result = mix (valuel, valuer, clamp(texcoord.x, 0.0, 1.0) );
}
