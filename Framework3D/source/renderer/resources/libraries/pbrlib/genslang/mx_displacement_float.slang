void mx_displacement_float(float disp, float scale, out displacementshader result)
{
    result.offset = float3(disp);
    result.scale = scale;
}
