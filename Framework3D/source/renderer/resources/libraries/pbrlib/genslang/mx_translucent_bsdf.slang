// We fake diffuse transmission by using diffuse reflection from the opposite side.
// So this BTDF is really a BRDF.
void mx_translucent_bsdf_reflection(float3 L, float3 V, float3 P, float occlusion, float weight, float3 color, float3 normal, inout BSDF bsdf)
{
    bsdf.throughput = float3(0.0);

    // Invert normal since we're transmitting light from the other side
    float NdotL = dot(L, -normal);
    if (NdotL <= 0.0 || weight < M_FLOAT_EPS)
    {
        return;
    }

    bsdf.response = color * weight * NdotL * M_PI_INV;
}

void mx_translucent_bsdf_indirect(float3 V, float weight, float3 color, float3 normal, inout BSDF bsdf)
{
    bsdf.throughput = float3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    // Invert normal since we're transmitting light from the other side
    float3 Li = mx_environment_irradiance(-normal);
    bsdf.response = Li * color * weight;
}
