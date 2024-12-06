RWTexture2D<float4> outputTexture;
StructuredBuffer<float> bufferR;
StructuredBuffer<float> bufferG;
StructuredBuffer<float> bufferB;
StructuredBuffer<float> bufferA;
ConstantBuffer<uint2> image_size;

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
    if (DTid.x >= image_size.x || DTid.y >= image_size.y) {
        return;
    }

    uint index = DTid.y * image_size.x + DTid.x;

    float4 color = float4(0, 0, 0, 0);
    color.r = bufferR[index];
    color.g = bufferG[index];
    color.b = bufferB[index];
    color.a = bufferA[index];

    color.a = 1;

    outputTexture[DTid.xy] += color;
}
