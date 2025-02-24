void mx_creatematrix_vector3_matrix33(float3 in1, float3 in2, float3 in3, out float3x3 result)
{
    result = float3x3(in1.x, in1.y, in1.z,
                  in2.x, in2.y, in2.z,
                  in3.x, in3.y, in3.z);
}
