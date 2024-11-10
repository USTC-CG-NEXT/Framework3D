#include "utils/view_cb.h"

// ---[ Constant Buffers ]---
ConstantBuffer<PlanarViewConstants> viewConstant : register(b0);

// Using a StructuredBuffer to hold the model matrices
StructuredBuffer<float4x4> modelMatrixBuffer : register(t0);

// Function to get the model matrix for a given instance
float4x4 GetModelMatrix(uint instanceID)
{
    return modelMatrixBuffer[instanceID];
}

// Vertex shader output structure
struct VSOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

VSOutput vs_main(
    float3 position: POSITION,
    float3 normal: NORMAL,
    uint instanceID: SV_InstanceID)
{
    VSOutput output;
    float4x4 modelMatrix = GetModelMatrix(instanceID);
    float4 worldPosition = mul(modelMatrix, float4(position, 1.0));
    float4 viewPosition = mul(viewConstant.matWorldToView, worldPosition);
    output.position = mul(viewConstant.matViewToClip, viewPosition);
    output.position = float4(position, 1.0);
    output.normal = mul((float3x3)modelMatrix, normal);
    return output;
}

float4 ps_main(VSOutput input) : SV_TARGET
{
    float3 lightDir = normalize(float3(0.5, 0.5, -1.0));
    float3 normal = normalize(input.normal);
    float diffuse = max(dot(normal, lightDir), 0.0);
    return float4(1, diffuse, 1, 1.0);
}
