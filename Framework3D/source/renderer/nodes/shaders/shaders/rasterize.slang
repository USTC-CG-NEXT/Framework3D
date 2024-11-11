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
#if defined(ENABLE_TEXCOORD)
    float2 texcoord : TEXCOORD0;
#endif
};

// Vertex shader output structure
struct VSOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION;

#if defined(ENABLE_TEXCOORD)
    float2 texcoord : TEXCOORD0;
#endif
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
    o_output.worldPos = worldPos.xyz;
    // Pass through the normal
    o_output.position.y *= -1;

    o_output.normal =
        normalize(mul(float4(i_input.normal, 0.0), modelMatrix).xyz);

#if defined(ENABLE_TEXCOORD)
    o_output.texcoord = i_input.texcoord;
#endif
}

void ps_main(
    in VSOutput input,
    out float4 o_pos: SV_Target0,
    out float2 o_texcoord: SV_Target1,
    out float4 o_diffuse_color: SV_Target2,
    out float4 o_metallic_roughness: SV_Target3,
    out float4 o_normal: SV_Target4,
    out float depth: SV_Depth)
{
    // Simply output the color passed from the vertex shader
    o_pos = float4(input.worldPos, 1.0);
    o_normal = float4(input.normal, 1.0);
    depth = input.position.z;
    o_diffuse_color = float4(1, 0, 1, 1);
    o_metallic_roughness = float4(0.5, 0.5, 0, 1);

#if defined(ENABLE_TEXCOORD)
    o_texcoord = input.texcoord;
#endif
    // Write depth using the position
}
