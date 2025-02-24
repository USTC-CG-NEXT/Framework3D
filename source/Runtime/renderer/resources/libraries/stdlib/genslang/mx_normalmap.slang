void mx_normalmap_vector2(float3 value, int map_space, float2 normal_scale, float3 N, float3 T,  out float3 result)
{
    // Decode the normal map.
    value = (value == float3(0.0f)) ? float3(0.0, 0.0, 1.0) : value * 2.0 - 1.0;

    // Transform from tangent space if needed.
    if (map_space == 0)
    {
        float3 B = normalize(cross(N, T));
        value.xy *= normal_scale;
        value = T * value.x + B * value.y + N * value.z;
    }

    // Normalize the result.
    result = normalize(value);
}

void mx_normalmap_float(float3 value, int map_space, float normal_scale, float3 N, float3 T,  out float3 result)
{
    mx_normalmap_vector2(value, map_space, float2(normal_scale), N, T, result);
}
