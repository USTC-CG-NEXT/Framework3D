RWStructuredBuffer<float> bufferR;
RWStructuredBuffer<float> bufferG;
RWStructuredBuffer<float> bufferB;
RWStructuredBuffer<float> bufferA;
StructuredBuffer<float4> inputColor;
StructuredBuffer<uint2> inputPixelID;
ConstantBuffer<uint> bufferLength;
ConstantBuffer<uint2> image_size;

[numthreads(64, 1, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
    if (DTid.x >= bufferLength) {
        return;
    }
    uint2 pixelID = inputPixelID[DTid.x];
    float4 color = inputColor[DTid.x];

    uint index = pixelID.y * image_size.x +
                 pixelID.x;  // Assuming you have the width of the texture

    InterlockedAdd(bufferR[index], color.r);
    InterlockedAdd(bufferG[index], color.g);
    InterlockedAdd(bufferB[index], color.b);
    InterlockedAdd(bufferA[index], color.a);
}
