import utils.Color.ColorHelpers;

RWTexture2D<float4> SourceTexture;
RWTexture2D<float4> MappedTexture;

ConstantBuffer<uint2> ImageSize;

[numthreads(8, 8, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
    uint2 coord = DTid.xy;
    if (coord.x >= ImageSize.x || coord.y >= ImageSize.y)
        return;
    float3 color = SourceTexture[coord].xyz;
    color = linearToSRGB(color);
    MappedTexture[coord] = float4(color, 1);
}
