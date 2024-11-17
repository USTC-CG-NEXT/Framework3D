void mx_transformmatrix_vector2M3(float2 val, mat3 transform, out float2 result)
{
  float3 res = transform * float3(val, 1.0);
  result = res.xy;
}
