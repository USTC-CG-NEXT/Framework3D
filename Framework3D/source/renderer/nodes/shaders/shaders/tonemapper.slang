// Adaptive Tone Mapping Compute Shader in Slang

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);

cbuffer cbImageSize : register(b0)
{
    uint2 gImageSize;
}

cbuffer cbToneMappingParams : register(b1)
{
    float gColorScale;
}

groupshared float groupMinLuminance[1];
groupshared float groupMaxLuminance[1];

#define FLT_MAX 3.402823466e+38

[numthreads(16, 16, 1)]
void main(
    uint3 dispatchThreadID: SV_DispatchThreadID,
    uint3 groupThreadID: SV_GroupThreadID)
{
    uint2 coord = dispatchThreadID.xy;

    if (coord.x >= gImageSize.x || coord.y >= gImageSize.y)
        return;

    // Initialize global min and max luminance
    float globalMinLuminance = FLT_MAX;
    float globalMaxLuminance = -FLT_MAX;

    // Sample 256 points across the entire texture
    for (uint i = 0; i < 256; ++i)
    {
        uint2 sampleCoord = uint2(
            (i * 9973) % gImageSize.x, // Use a large prime number for pseudo-random sampling
            (i * 7919) % gImageSize.y  // Use a different large prime number for pseudo-random sampling
        );

        float4 color = gInputTexture.Load(int3(sampleCoord, 0));
        float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722)); // Calculate luminance

        globalMinLuminance = min(globalMinLuminance, luminance);
        globalMaxLuminance = max(globalMaxLuminance, luminance);
    }

    // Store global results in group shared memory
    if (groupThreadID.x == 0 && groupThreadID.y == 0)
    {
        groupMinLuminance[0] = globalMinLuminance;
        groupMaxLuminance[0] = globalMaxLuminance;
    }

    GroupMemoryBarrierWithGroupSync();

    // Apply tone mapping
    float minLuminance = groupMinLuminance[0];
    float maxLuminance = groupMaxLuminance[0];
    float4 color = gInputTexture.Load(int3(coord, 0));
    float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722)); // Calculate luminance
    float mappedLuminance = (luminance - minLuminance) / (maxLuminance - minLuminance + 1e-5);
    float3 mappedColor = color.rgb * (mappedLuminance / luminance) * gColorScale;
    gOutputTexture[coord] = float4(mappedColor, 1.0);
}
