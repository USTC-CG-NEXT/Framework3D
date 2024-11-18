void mx_ramptb_float(float valuet, float valueb, float2 texcoord, out float result)
{
    result = lerp (valuet, valueb, clamp(texcoord.y, 0.0, 1.0) );
}
