#include "lib/mx_microfacet_specular.slang"

void mx_generalized_schlick_bsdf_reflection(float3 L, float3 V, float3 P, float occlusion, float weight, float3 color0, float3 color90, float exponent, float2 roughness, float3 N, float3 X, int distribution, int scatter_mode, inout BSDF bsdf)
{
    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    N = mx_forward_facing_normal(N, V);

    X = normalize(X - dot(X, N) * N);
    float3 Y = cross(N, X);
    float3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

    float2 safeAlpha = clamp(roughness, M_FLOAT_EPS, 1.0);
    float avgAlpha = mx_average_alpha(safeAlpha);
    float3 Ht = float3(dot(H, X), dot(H, Y), dot(H, N));

    FresnelData fd;
    float3 safeColor0 = max(color0, 0.0);
    float3 safeColor90 = max(color90, 0.0);
    if (bsdf.thickness > 0.0)
    { 
        fd = mx_init_fresnel_schlick_airy(safeColor0, safeColor90, exponent, bsdf.thickness, bsdf.ior);
    }
    else
    {
        fd = mx_init_fresnel_schlick(safeColor0, safeColor90, exponent);
    }
    float3  F = mx_compute_fresnel(VdotH, fd);
    float D = mx_ggx_NDF(Ht, safeAlpha);
    float G = mx_ggx_smith_G2(NdotL, NdotV, avgAlpha);

    float3 comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
    float3 dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, safeColor0, safeColor90) * comp;
    float avgDirAlbedo = dot(dirAlbedo, float3(1.0 / 3.0));
    bsdf.throughput = float3(1.0 - avgDirAlbedo * weight);

    // Note: NdotL is cancelled out
    bsdf.response = D * F * G * comp * occlusion * weight / (4.0 * NdotV);
}

void mx_generalized_schlick_bsdf_transmission(float3 V, float weight, float3 color0, float3 color90, float exponent, float2 roughness, float3 N, float3 X, int distribution, int scatter_mode, inout BSDF bsdf)
{
    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    N = mx_forward_facing_normal(N, V);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    FresnelData fd;
    float3 safeColor0 = max(color0, 0.0);
    float3 safeColor90 = max(color90, 0.0);
    if (bsdf.thickness > 0.0)
    { 
        fd = mx_init_fresnel_schlick_airy(safeColor0, safeColor90, exponent, bsdf.thickness, bsdf.ior);
    }
    else
    {
        fd = mx_init_fresnel_schlick(safeColor0, safeColor90, exponent);
    }
    float3 F = mx_compute_fresnel(NdotV, fd);

    float2 safeAlpha = clamp(roughness, M_FLOAT_EPS, 1.0);
    float avgAlpha = mx_average_alpha(safeAlpha);

    float3 comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
    float3 dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, safeColor0, safeColor90) * comp;
    float avgDirAlbedo = dot(dirAlbedo, float3(1.0 / 3.0));
    bsdf.throughput = float3(1.0 - avgDirAlbedo * weight);

    if (scatter_mode != 0)
    {
        float avgF0 = dot(safeColor0, float3(1.0 / 3.0));
        fd.ior = float3(mx_f0_to_ior(avgF0));
        bsdf.response = mx_surface_transmission(N, V, X, safeAlpha, distribution, fd, safeColor0) * weight;
    }
}

void mx_generalized_schlick_bsdf_indirect(float3 V, float weight, float3 color0, float3 color90, float exponent, float2 roughness, float3 N, float3 X, int distribution, int scatter_mode, inout BSDF bsdf)
{
    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    N = mx_forward_facing_normal(N, V);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    FresnelData fd;
    float3 safeColor0 = max(color0, 0.0);
    float3 safeColor90 = max(color90, 0.0);
    if (bsdf.thickness > 0.0)
    { 
        fd = mx_init_fresnel_schlick_airy(safeColor0, safeColor90, exponent, bsdf.thickness, bsdf.ior);
    }
    else
    {
        fd = mx_init_fresnel_schlick(safeColor0, safeColor90, exponent);
    }
    float3 F = mx_compute_fresnel(NdotV, fd);

    float2 safeAlpha = clamp(roughness, M_FLOAT_EPS, 1.0);
    float avgAlpha = mx_average_alpha(safeAlpha);
    float3 comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
    float3 dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, safeColor0, safeColor90) * comp;
    float avgDirAlbedo = dot(dirAlbedo, float3(1.0 / 3.0));
    bsdf.throughput = float3(1.0 - avgDirAlbedo * weight);

    float3 Li = mx_environment_radiance(N, V, X, safeAlpha, distribution, fd);
    bsdf.response = Li * comp * weight;
}
