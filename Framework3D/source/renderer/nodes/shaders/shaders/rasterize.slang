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

struct VS_INPUT {
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

// Vertex shader output structure
struct VSOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 color : COLOR;
};
void vs_main(
    in VS_INPUT i_input,
    uint i_vertexID: SV_VertexID,
    uint i_instanceID: SV_InstanceID,
    out VSOutput o_output)
{
    // Get the model matrix for the current instance
    float4x4 modelMatrix = GetModelMatrix(i_instanceID);

    // Transform the vertex position to world space
    float4 worldPos = mul(float4(i_input.pos, 1.0), modelMatrix);

    // Transform the vertex position to clip space
    o_output.position = mul(worldPos, viewConstant.matWorldToClip);

    // Pass through the normal
    o_output.position.y *= -1;

    // Assign color based on vertex ID
    o_output.color = i_input.normal.xyz * 0.5 + 0.5f;
}

void ps_main(
    in VSOutput input,
    out float4 o_color: SV_Target0,
    out float depth: SV_Depth)
{
    // Simply output the color passed from the vertex shader
    depth = input.position.z;
    o_color = float4(input.color, 1.0);

    // Write depth using the position
}
