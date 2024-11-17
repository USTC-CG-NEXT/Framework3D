#include "mx_microfacet_specular.slang"

float3 mx_environment_radiance(float3 N, float3 V, float3 X, float2 roughness, int distribution, FresnelData fd)
{
    return float3(0.0);
}

float3 mx_environment_irradiance(float3 N)
{
    return float3(0.0);
}
