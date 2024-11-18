void mx_transformmatrix_vector3M4(float3 val, float4x4 transform, out float3 result)
{
  float4 res = transform * float4(val, 1.0);
  result = res.xyz;
}
