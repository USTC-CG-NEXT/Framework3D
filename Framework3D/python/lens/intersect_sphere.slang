RayInfo intersect_sphere(
    RayInfo ray,
    inout float3 weight,
    inout float t,
    float radius,
    float center_pos,
    float alpha_range,
    float refractive_index,
    float abbe_number)
{
    float3 ray_dir = ray.Direction;
    float3 ray_pos = ray.Origin;
    float3 sphere_center = float3(0, 0, center_pos);

    // Calculate the vector from the ray origin to the sphere center
    float3 oc = ray_pos - sphere_center;

    // Calculate the coefficients of the quadratic equation
    float a = dot(ray_dir, ray_dir);
    float b = 2.0 * dot(oc, ray_dir);
    float c = dot(oc, oc) - radius * radius;

    // Calculate the discriminant
    float discriminant = b * b - 4.0 * a * c;

    // Check if the ray intersects the sphere
    if (discriminant < 0.0) {
        // No intersection
        t = 0;
        weight = 0;
        return RayInfo(ray_pos, 0, ray_dir, 1000.0);
    }

    // Calculate the distance to the intersection points
    float t1 = (-b - sqrt(discriminant)) / (2.0 * a);
    float t2 = (-b + sqrt(discriminant)) / (2.0 * a);

    // Choose the closest positive intersection
    t = (t1 > 0.0) ? t1 : t2;

    // Calculate the intersection position
    float3 intersection_pos = ray_pos + t * ray_dir;

    // Calculate the normal at the intersection point
    float3 normal = normalize(intersection_pos - sphere_center);
    if (radius < 0)
        normal *= -1;

    // Calculate the refracted direction using Snell's law
    float eta = refractive_index;
    float3 refracted_dir = normalize(refract(ray_dir, normal, eta));
    // Calculate the angle of the intersection point to the normalized sphere
    // center
    float3 normalized_sphere_center = normalize(-sphere_center);
    float angle = acos(abs(normal.z));

    // Check if the angle is within the allowed range
    if (angle > alpha_range) {
        // No valid intersection within the alpha range
        return RayInfo(ray_pos, 0, ray_dir, 0.0);
        weight = 0;
    }

    return RayInfo(intersection_pos, 0, refracted_dir, 1000.f);
}
