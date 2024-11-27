import utils.ray;

[Differentiable]
RayInfo intersect_occluder(
    RayInfo ray,
    inout float3 weight,
    inout float t,
    float radius,
    float center_pos)
{
    float3 ray_dir = ray.Direction;
    float3 ray_pos = ray.Origin;
    float3 plane_center = float3(0, 0, center_pos);

    // Calculate the vector from the ray origin to the plane center
    float3 oc = ray_pos - plane_center;

    // Calculate the distance to the intersection point
    t = dot(plane_center - ray_pos, float3(0, 0, 1)) /
        dot(ray_dir, float3(0, 0, 1));

    // Calculate the intersection position
    float3 intersection_pos = ray_pos + t * ray_dir;

    if (length(intersection_pos.xy) > radius) {
        weight = 0;
    }

    return RayInfo(intersection_pos, 0, ray_dir, 1000.f);
}
