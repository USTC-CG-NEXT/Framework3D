
import Scene.SceneTypes;
import Scene.Scene;

[[vk::binding(0, 1)]]
StructuredBuffer<float> t_BindlessBuffers[] : register(t0, space1);

StructuredBuffer<uint> index_buffer;
StructuredBuffer<GeometryInstanceData> instanceDescBuffer;
StructuredBuffer<MeshDesc> meshDescBuffer;

StaticVertexData load_vertex(MeshDesc mesh_data, uint vertex_id)
{
    var vb_offset = mesh_data.vbOffset;
    var normal_offset = mesh_data.normalOffset;

    // normal_offset = 0;

    var id = vertex_id;

    float3 pos = float3(
        t_BindlessBuffers[0][3 * vertex_id + vb_offset + 0],
        t_BindlessBuffers[0][3 * vertex_id + vb_offset + 1],
        t_BindlessBuffers[0][3 * vertex_id + vb_offset + 2]);

    float3 normal = float3(
        t_BindlessBuffers[0][3 * vertex_id + normal_offset + 0],
        t_BindlessBuffers[0][3 * vertex_id + normal_offset + 1],
        t_BindlessBuffers[0][3 * vertex_id + normal_offset + 2]);

    StaticVertexData data =
        StaticVertexData(pos, normal, float4(0, 0, 0, 0), float2(0, 0), 0);
    return data;
}

VertexData
get_interpolated_vertex(uint instance_id, uint primitive_id, float2 baricentric2)
{
    GeometryInstanceData instance = instanceDescBuffer[instance_id];
    MeshDesc mesh = meshDescBuffer[instance.geometryID];
    var id_in_indexbuffer = mesh.ibOffset + primitive_id * 3;

    uint i0 = index_buffer[id_in_indexbuffer + 0];
    uint i1 = index_buffer[id_in_indexbuffer + 1];
    uint i2 = index_buffer[id_in_indexbuffer + 2];

    StaticVertexData v[3];

    v[0] = load_vertex(mesh, i0);
    v[1] = load_vertex(mesh, i1);
    v[2] = load_vertex(mesh, i2);

    float4x4 worldMat = transpose(instance.transform);
    float3x3 worldInvTransposeMat = transpose(inverse(float3x3(worldMat)));

    float3 baricentric =
        float3(1.0 - baricentric2.x - baricentric2.y, baricentric2.x, baricentric2.y);
    return computeVertexData(v, worldMat, worldInvTransposeMat, false, baricentric);
}
