#include "mx_microfacet_specular.slang"

// Construct an orthonormal basis from a unit floattor.
// https://graphics.pixar.com/library/OrthonormalB/paper.pdf
float3x3 mx_orthonormal_basis(float3 N)
{
    float sign = (N.z < 0.0) ? -1.0 : 1.0;
    float a = -1.0 / (sign + N.z);
    float b = N.x * N.y * a;
    float3 X = float3(1.0 + sign * N.x * N.x * a, sign * b, -sign * N.x);
    float3 Y = float3(b, sign + N.y * N.y * a, -N.y);
    return float3x3(X, Y, N);
}

// Return the alpha associated with the given mip level in a prefiltered environment.
float mx_latlong_lod_to_alpha(float lod)
{
    float lodBias = lod / float(u_envRadianceMips - 1);
    return (lodBias < 0.5) ? mx_square(lodBias) : 2.0 * (lodBias - 0.375);
}

// The inverse of mx_latlong_projection.
float3 mx_latlong_map_projection_inverse(float2 uv)
{
    float latitude = (uv.y - 0.5) * M_PI;
    float longitude = (uv.x - 0.5) * M_PI * 2.0;

    float x = -cos(latitude) * sin(longitude);
    float y = -sin(latitude);
    float z = cos(latitude) * cos(longitude);

    return float3(x, y, z);
}

float3 mx_generate_prefilter_env()
{
    // The tangent view floattor is aligned with the normal.
    float3 V = float3(0.0, 0.0, 1.0);
    float NdotV = 1.0;

    // Compute derived properties.
    float2 uv = gl_FragCoord.xy * pow(2.0, u_envPrefilterMip) / float2(textureSize(u_envRadiance, 0));
    float3 worldN = mx_latlong_map_projection_inverse(uv);
    float3x3 tangentToWorld = mx_orthonormal_basis(worldN);
    float alpha = mx_latlong_lod_to_alpha(float(u_envPrefilterMip));
    float G1V = mx_ggx_smith_G1(NdotV, alpha);

    // Integrate the LD term for the given environment and alpha.
    float3 radiance = float3(0.0, 0.0, 0.0);
    float weight = 0.0;
    int envRadianceSamples = 1024;
    for (int i = 0; i < envRadianceSamples; i++)
    {
        float2 Xi = mx_spherical_fibonacci(i, envRadianceSamples);

        // Compute the half floattor and incoming light direction.
        float3 H = mx_ggx_importance_sample_VNDF(Xi, V, float2(alpha));
        float3 L = -V + 2.0 * H.z * H;

        // Compute dot products for this sample.
        float NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);

        // Compute the geometric term.
        float G = mx_ggx_smith_G2(NdotL, NdotV, alpha);

        // Sample the environment light from the given direction.
        float3 Lw = tangentToWorld * L;
        float pdf = mx_ggx_NDF(H, float2(alpha)) * G1V / (4.0 * NdotV);
        float lod = mx_latlong_compute_lod(Lw, pdf, float(u_envRadianceMips - 1), envRadianceSamples);
        float3 sampleColor = mx_latlong_map_lookup(Lw, u_envMatrix, lod, u_envRadiance);

        // Add the radiance contribution of this sample.
        radiance += G * sampleColor;
        weight += G;
    }

    return radiance / weight;
}
