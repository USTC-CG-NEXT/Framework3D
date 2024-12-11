#include "utils/view_cb.h"
import Scene.SceneTypes;

// ---[ Constant Buffers ]---
ConstantBuffer<PlanarViewConstants> viewConstant;

StructuredBuffer<GeometryInstanceData> instanceDescBuffer;
StructuredBuffer<MeshDesc> meshDescBuffer;

[[vk::binding(0, 1)]]
StructuredBuffer<float> t_BindlessBuffers[] : register(t0, space1);

// Vertex shader output structure
struct VSOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION;

#if defined(ENABLE_TEXCOORD)
    float2 texcoord : TEXCOORD0;
#endif
};

StaticVertexData load_vertex(MeshDesc mesh_data, uint vertex_id)
{
    var vb_offset = mesh_data.vbOffset;
    var normal_offset = mesh_data.normalOffset;

    float3 pos = float3(
        t_BindlessBuffers[mesh_data.vbBufferIndex][vertex_id + vb_offset + 0],
        t_BindlessBuffers[mesh_data.vbBufferIndex][vertex_id + vb_offset + 1],
        t_BindlessBuffers[mesh_data.vbBufferIndex][vertex_id + vb_offset + 2]);

    float3 normal = float3(
        t_BindlessBuffers[mesh_data.vbBufferIndex]
                         [vertex_id + normal_offset + 0],
        t_BindlessBuffers[mesh_data.vbBufferIndex]
                         [vertex_id + normal_offset + 1],
        t_BindlessBuffers[mesh_data.vbBufferIndex]
                         [vertex_id + normal_offset + 2]);

    StaticVertexData data =
        StaticVertexData(pos, normal, float4(0, 0, 0, 0), float2(0, 0), 0);
    return data;
}

void vs_main(
    uint i_vertexID: SV_VertexID,
    uint i_instanceID: SV_InstanceID,
    out VSOutput o_output)
{
    var InstanceData = instanceDescBuffer[i_instanceID];
    var MeshData = meshDescBuffer[InstanceData.geometryID];
    // Get the model matrix for the current instance
    float4x4 modelMatrix = InstanceData.transform;

    StaticVertexData vertex = load_vertex(MeshData, i_vertexID);

    // Get the vertex position from the bindless buffer

    // Transform the vertex position to world space
    float4 worldPos = mul(float4(vertex.position, 1.0), modelMatrix);
    worldPos = float4(vertex.position, 1.0);

    // Transform the vertex position to clip space
    o_output.position = mul(worldPos, viewConstant.matWorldToClip);
    o_output.worldPos = worldPos.xyz;
    // Pass through the normal
    o_output.position.y *= -1;

    o_output.normal =
        normalize(mul(float4(vertex.normal, 0.0), modelMatrix).xyz);

#if defined(ENABLE_TEXCOORD)
    o_output.texcoord = vertex.texCrd;
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
    o_normal = float4(1, 1, 1, 1.0);
    depth = input.position.z;
    o_diffuse_color = float4(1, 0, 1, 1);
    o_metallic_roughness = float4(0.5, 0.5, 0, 1);

#if defined(ENABLE_TEXCOORD)
    o_texcoord = input.texcoord;
#endif
    // Write depth using the position
}
