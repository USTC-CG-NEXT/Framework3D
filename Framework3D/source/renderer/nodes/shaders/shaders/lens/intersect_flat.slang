import utils.ray;

[Differentiable]
RayInfo intersect_flat(
    RayInfo ray,
    inout float t,
    float diameter,
    float center_pos,
    float refractive_index,
    float abbe_number)
{
    float3 ray_dir = ray.Direction;
    float3 ray_pos = ray.Origin;
    float3 plane_center = float3(0, 0, center_pos);

    // Calculate the vector from the ray origin to the plane center
    float3 oc = ray_pos - plane_center;

    // Calculate the distance to the intersection point
    t = dot(plane_center - ray_pos, float3(0, 0, 1)) /
        dot(ray_dir, float3(0, 0, 1));

    if (t > ray.TMax) {
        t = 0;

        return RayInfo(ray_pos, 0, ray_dir, 0.0, { 0 });
    }

    // Calculate the intersection position
    float3 intersection_pos = ray_pos + t * ray_dir;

    // Calculate the normal at the intersection point
    float3 normal = float3(0, 0, -1);

    // Calculate the refracted direction using Snell's law
    float eta = refractive_index;
    float3 refracted_dir;
    if (safe_refract(ray_dir, normal, eta, refracted_dir))
        refracted_dir = normalize(refracted_dir);
    else {
        ray.throughput = { 0 };
        return RayInfo(ray_pos, 0, ray_dir, 0.0, { 0 });
    }

    return RayInfo(intersection_pos, 0, refracted_dir, 1000.f);
}
