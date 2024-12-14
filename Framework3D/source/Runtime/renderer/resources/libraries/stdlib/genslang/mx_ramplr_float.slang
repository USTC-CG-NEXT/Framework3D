void mx_ramplr_float(float valuel, float valuer, float2 texcoord, out float result)
{
    result = lerp (valuel, valuer, clamp(texcoord.x, 0.0, 1.0) );
}
