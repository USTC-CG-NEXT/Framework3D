RWTexture2DArray<float> textures;
RWTexture2D<float4> outputTexture;

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
    uint width;
    uint height;
    outputTexture.GetDimensions(width, height);

    if (DTid.x >= width || DTid.y >= height) {
        return;
    }

    float4 color = float4(0, 0, 0, 0);
    color.r = textures[uint3(DTid.x, DTid.y, 0)];
    color.g = textures[uint3(DTid.x, DTid.y, 1)];
    color.b = textures[uint3(DTid.x, DTid.y, 2)];
    color.a = textures[uint3(DTid.x, DTid.y, 3)];

    color.a = 1;

    outputTexture[DTid.xy] += color;
}
