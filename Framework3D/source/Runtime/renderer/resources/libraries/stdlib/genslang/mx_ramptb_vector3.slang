void mx_ramptb_vector3(float3 valuet, float3 valueb, float2 texcoord, out float3 result)
{
    result = lerp (valuet, valueb, clamp(texcoord.y, 0.0, 1.0) );
}
