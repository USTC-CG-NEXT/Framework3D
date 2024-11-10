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

void vs_main(
    float3 i_pos: POSITION,
    float3 normal: NORMAL,
    uint i_vertexID: SV_VertexID,
    uint i_instanceID: SV_InstanceID,
    out float4 o_pos: SV_Position,
    out float3 o_color: COLOR)
{
    // Get the model matrix for the current instance
    float4x4 modelMatrix = GetModelMatrix(i_instanceID);

    // Transform the vertex position to world space
    float4 worldPos = mul(modelMatrix, float4(i_pos, 1.0));

    // Transform the vertex position to view space
    float4 viewPos = mul(viewConstant.matWorldToView, worldPos);

    // Transform the vertex position to clip space
    // o_pos = mul(viewConstant.matWorldToClip, worldPos);
    // o_pos = mul(viewConstant.matClipToWorld, worldPos);
    o_pos = mul(viewConstant.matWorldToClip, worldPos);
    o_pos = float4(-i_pos * 0.5, 1.0);
    o_pos.xy *= 10;
    // o_pos = mul(transpose(viewConstant.matClipToWorld), worldPos);

    //    o_pos = mul(worldPos, viewConstant.matWorldToClip);
    float4 ret = abs(float4(-i_pos*0.5, 1.0));
    // o_pos.w);
    // For debugging, set o_pos to the corners
    if (i_vertexID == 0) {
        o_pos += float4(-1.0, -1.0, 0.0, 1.0);
    } else if (i_vertexID == 1) {
        o_pos += float4(1.0, -1.0, 0.0, 1.0);
    } else if (i_vertexID == 2) {
        o_pos += float4(-1.0, 1.0, 0.0, 1.0);
    } else if (i_vertexID == 3) {
        o_pos = o_pos*2+ float4(1.0, 1.0, 0.0, 1.0);
    }


    

    // Pass the normal through as color for now
    o_color = (ret.xyz / ret.w);
}

void ps_main(
    in float4 i_pos: SV_Position,
    out float4 o_color: SV_Target,
    in float3 i_color: COLOR)
{
    // Simply output the color passed from the vertex shader
    o_color = float4((i_color), 1.0);
}
