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

groupshared float3 groupMinColor[1];
groupshared float3 groupMaxColor[1];
groupshared float3 sharedMin[256];
groupshared float3 sharedMax[256];

[numthreads(16, 16, 1)]
void main(
    uint3 dispatchThreadID: SV_DispatchThreadID,
    uint3 groupThreadID: SV_GroupThreadID)
{
    uint2 coord = dispatchThreadID.xy;
    uint localIndex = groupThreadID.x + groupThreadID.y * 16;

    if (coord.x >= gImageSize.x || coord.y >= gImageSize.y)
        return;

    float4 color = gInputTexture.Load(int3(coord, 0));
    float3 rgb = color.rgb;

    // Initialize local min and max
    float3 localMin = rgb;
    float3 localMax = rgb;

    sharedMin[localIndex] = localMin;
    sharedMax[localIndex] = localMax;

    GroupMemoryBarrierWithGroupSync();

    // Reduction to find group min and max
    for (uint stride = 256 / 2; stride > 0; stride /= 2) {
        if (localIndex < stride) {
            sharedMin[localIndex] =
                min(sharedMin[localIndex], sharedMin[localIndex + stride]);
            sharedMax[localIndex] =
                max(sharedMax[localIndex], sharedMax[localIndex + stride]);
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // Store group results
    if (localIndex == 0) {
        groupMinColor[0] = sharedMin[0];
        groupMaxColor[0] = sharedMax[0];
    }

    GroupMemoryBarrierWithGroupSync();

    // Apply tone mapping
    float3 minColor = groupMinColor[0];
    float3 maxColor = groupMaxColor[0];
    float3 mappedColor = (rgb - minColor) / (maxColor - minColor + 1e-5);
    mappedColor = rgb * gColorScale;
    gOutputTexture[coord] = float4(mappedColor, color.a);
}
