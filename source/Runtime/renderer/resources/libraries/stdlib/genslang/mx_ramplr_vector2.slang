void mx_ramplr_vector2(float2 valuel, float2 valuer, float2 texcoord, out float2 result)
{
    result = lerp (valuel, valuer, clamp(texcoord.x, 0.0, 1.0) );
}
