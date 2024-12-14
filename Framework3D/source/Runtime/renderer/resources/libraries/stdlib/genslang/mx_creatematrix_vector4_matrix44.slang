void mx_creatematrix_vector4_matrix44(float4 in1, float4 in2, float4 in3, float4 in4, out float4x4 result)
{
    result = float4x4(in1.x, in1.y, in1.z, in1.w,
                  in2.x, in2.y, in2.z, in2.w,
                  in3.x, in3.y, in3.z, in3.w,
                  in4.x, in4.y, in4.z, in4.w);
}
