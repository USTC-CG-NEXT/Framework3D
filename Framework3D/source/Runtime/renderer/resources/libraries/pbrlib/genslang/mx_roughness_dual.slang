void mx_roughness_dual(float2 roughness, out float2 result)
{
    if (roughness.y < 0.0)
    {
        roughness.y = roughness.x;
    }
    result.x = clamp(roughness.x * roughness.x, M_FLOAT_EPS, 1.0);
    result.y = clamp(roughness.y * roughness.y, M_FLOAT_EPS, 1.0);
}
