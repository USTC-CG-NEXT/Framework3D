void mx_creatematrix_vector3_matrix44(float3 in1, float3 in2, float3 in3, float3 in4, out float4x4 result)
{
    result = float4x4(in1.x, in1.y, in1.z, 0.0,
                  in2.x, in2.y, in2.z, 0.0,
                  in3.x, in3.y, in3.z, 0.0,
                  in4.x, in4.y, in4.z, 1.0);
}
