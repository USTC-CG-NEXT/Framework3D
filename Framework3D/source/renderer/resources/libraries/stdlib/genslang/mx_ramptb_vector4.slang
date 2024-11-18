void mx_ramptb_vector4(float4 valuet, float4 valueb, float2 texcoord, out float4 result)
{
    result = lerp (valuet, valueb, clamp(texcoord.y, 0.0, 1.0) );
}
