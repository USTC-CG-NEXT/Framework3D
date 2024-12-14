[Differentiable]
float get_relative_refractive_index(
    float refract_id_last,
    float refract_id_this)
{
    return 1.0f / (refract_id_this / refract_id_last);
}
