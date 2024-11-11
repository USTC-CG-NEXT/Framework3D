Texture2D<float4> WorldPosition;
Texture2D<float4> WorldDirection;
ConstantBuffer<uint2> size;

RWStructuredBuffer<float4x4> Transforms;
RWStructuredBuffer<uint> Counter;

[numthreads(32, 32, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
    if (DTid.x >= size.x || DTid.y >= size.y)
        return;

    if (DTid.x % 16 == 0 && DTid.y % 16 == 0) {
        uint id;
        InterlockedAdd(Counter[0], 1, id);

        float3 worldPos = WorldPosition.Load(int3(DTid.xy, 0)).xyz;
        float3 worldDir = normalize(WorldDirection.Load(int3(DTid.xy, 0)).xyz);

        float4x4 transform;

        
        float3x3 rotationMatrix;
        float3 xAxis = worldDir;
        float3 yAxis = float3(0, 1, 0);
        float3 zAxis = cross(xAxis, yAxis);
        yAxis = cross(zAxis, xAxis);

        rotationMatrix[0] = normalize(xAxis);
        rotationMatrix[1] = normalize(yAxis);
        rotationMatrix[2] = normalize(zAxis);

        transform = float4x4(
            rotationMatrix[0].x, rotationMatrix[0].y, rotationMatrix[0].z, 0,
            rotationMatrix[1].x, rotationMatrix[1].y, rotationMatrix[1].z, 0,
            rotationMatrix[2].x, rotationMatrix[2].y, rotationMatrix[2].z, 0,
            worldPos.x, worldPos.y, worldPos.z, 1
        );

        transform[0] *= 0.06;
        transform[1] *= 0.06;
        transform[2] *= 0.06;
        Transforms[id] = transform;
    }
}
