#include "lib/mx_microfacet_diffuse.slang"

void mx_subsurface_bsdf_reflection(float3 L, float3 V, float3 P, float occlusion, float weight, float3 color, float3 radius, float anisotropy, float3 normal, inout BSDF bsdf)
{
    bsdf.throughput = float3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    float3 sss = mx_subsurface_scattering_approx(normal, L, P, color, radius);
    float NdotL = clamp(dot(normal, L), M_FLOAT_EPS, 1.0);
    float visibleOcclusion = 1.0 - NdotL * (1.0 - occlusion);
    bsdf.response = sss * visibleOcclusion * weight;
}

void mx_subsurface_bsdf_indirect(float3 V, float weight, float3 color, float3 radius, float anisotropy, float3 normal, inout BSDF bsdf)
{
    bsdf.throughput = float3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    // For now, we render indirect subsurface as simple indirect diffuse.
    float3 Li = mx_environment_irradiance(normal);
    bsdf.response = Li * color * weight;
}
