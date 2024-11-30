RWTexture2DArray<float> textures;
StructuredBuffer<float4> inputColor;
StructuredBuffer<uint2> inputPixelID;
ConstantBuffer<uint> bufferLength;

[numthreads(64, 1, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
    if (DTid.x >= bufferLength) {
        return;
    }
    uint2 pixelID = inputPixelID[DTid.x];
    float4 color = inputColor[DTid.x];

    InterlockedAdd(textures[uint3(pixelID, 0)], color.r);
    InterlockedAdd(textures[uint3(pixelID, 1)], color.g);
    InterlockedAdd(textures[uint3(pixelID, 2)], color.b);
    InterlockedAdd(textures[uint3(pixelID, 3)], color.a);
}
