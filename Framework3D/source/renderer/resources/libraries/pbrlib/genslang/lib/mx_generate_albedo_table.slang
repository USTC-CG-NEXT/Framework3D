#include "mx_microfacet_sheen.slang"
#include "mx_microfacet_specular.slang"

float3 mx_generate_dir_albedo_table()
{
    float2 uv = gl_FragCoord.xy / u_albedoTableSize;
    float2 ggxDirAlbedo = mx_ggx_dir_albedo(uv.x, uv.y, float3(1, 0, 0), float3(0, 1, 0)).xy;
    float sheenDirAlbedo = mx_imageworks_sheen_dir_albedo(uv.x, uv.y);
    return float3(ggxDirAlbedo, sheenDirAlbedo);
}
