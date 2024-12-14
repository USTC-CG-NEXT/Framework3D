#include "lib/mx_microfacet.slang"

void mx_generalized_schlick_edf(float3 N, float3 V, float3 color0, float3 color90, float exponent, EDF base, out EDF result)
{
    N = mx_forward_facing_normal(N, V);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float3 f = mx_fresnel_schlick(NdotV, color0, color90, exponent);
    result = base * f;
}
