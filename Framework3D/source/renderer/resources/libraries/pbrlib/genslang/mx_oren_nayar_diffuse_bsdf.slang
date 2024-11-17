#include "lib/mx_microfacet_diffuse.slang"

void mx_oren_nayar_diffuse_bsdf_reflection(float3 L, float3 V, float3 P, float occlusion, float weight, float3 color, float roughness, float3 normal, inout BSDF bsdf)
{
    bsdf.throughput = float3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    float NdotL = clamp(dot(normal, L), M_FLOAT_EPS, 1.0);

    bsdf.response = color * occlusion * weight * NdotL * M_PI_INV;
    if (roughness > 0.0)
    {
        bsdf.response *= mx_oren_nayar_diffuse(L, V, normal, NdotL, roughness);
    }
}

void mx_oren_nayar_diffuse_bsdf_indirect(float3 V, float weight, float3 color, float roughness, float3 normal, inout BSDF bsdf)
{
    bsdf.throughput = float3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    float3 Li = mx_environment_irradiance(normal);
    bsdf.response = Li * color * weight;
}
