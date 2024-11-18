void mx_ramptb_vector2(float2 valuet, float2 valueb, float2 texcoord, out float2 result)
{
    result = lerp (valuet, valueb, clamp(texcoord.y, 0.0, 1.0) );
}
