void mx_artistic_ior(float3 reflectivity, float3 edge_color, out float3 ior, out float3 extinction)
{
    // "Artist Friendly Metallic Fresnel", Ole Gulbrandsen, 2014
    // http://jcgt.org/published/0003/04/03/paper.pdf

    float3 r = clamp(reflectivity, 0.0, 0.99);
    float3 r_sqrt = sqrt(r);
    float3 n_min = (1.0 - r) / (1.0 + r);
    float3 n_max = (1.0 + r_sqrt) / (1.0 - r_sqrt);
    ior = lerp(n_max, n_min, edge_color);

    float3 np1 = ior + 1.0;
    float3 nm1 = ior - 1.0;
    float3 k2 = (np1*np1 * r - nm1*nm1) / (1.0 - r);
    k2 = max(k2, 0.0);
    extinction = sqrt(k2);
}
