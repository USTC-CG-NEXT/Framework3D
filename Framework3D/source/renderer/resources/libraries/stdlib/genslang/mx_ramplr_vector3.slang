void mx_ramplr_vector3(float3 valuel, float3 valuer, float2 texcoord, out float3 result)
{
    result = mix (valuel, valuer, clamp(texcoord.x, 0.0, 1.0) );
}
