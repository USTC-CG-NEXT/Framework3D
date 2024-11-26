
#include "utils/random.slangh"
#include "utils/Math/MathConstants.slangh"
#include "utils/ray.h"
import Utils.Math.MathHelpers;


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
    if(radius<0)
        normal*=-1;

    // Calculate the refracted direction using Snell's law
    float eta = refractive_index;
    float3 refracted_dir = normalize( refract(ray_dir, normal, eta));
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



RayInfo intersect_flat(
    RayInfo ray,
    inout float3 weight,
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
    t = dot(plane_center - ray_pos, float3(0, 0, 1)) / dot(ray_dir, float3(0, 0, 1));

    // Calculate the intersection position
    float3 intersection_pos = ray_pos + t * ray_dir;

    // Calculate the normal at the intersection point
    float3 normal = float3(0, 0, -1);

    // Calculate the refracted direction using Snell's law
    float eta = refractive_index;
    float3 refracted_dir = normalize(refract(ray_dir, normal, eta));

    return RayInfo(intersection_pos, 0, refracted_dir, 1000.f);
}


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
    t = dot(plane_center - ray_pos, float3(0, 0, 1)) / dot(ray_dir, float3(0, 0, 1));

    // Calculate the intersection position
    float3 intersection_pos = ray_pos + t * ray_dir;
    
    if(length(intersection_pos.xy)>radius)
    {
        weight = 0;
    }

    return RayInfo(intersection_pos, 0, ray_dir, 1000.f);
}


float get_relative_refractive_index(float refract_id_last, float refract_id_this)
{
	return 1.0f/(refract_id_this / refract_id_last);
}

struct LensSystemData
{
    float2 film_size;;
    int2 film_resolution;;
    float film_distance;;
    float optical_property_0_refractive_index;;
    float diameter_1;
    float radius_of_curvature_1;
    float theta_range_1;
    float sphere_center_1;
    float center_pos_1;
    float optical_property_1_refractive_index;;
    float optical_property_1_abbe_number;;
    float diameter_2;
    float radius_of_curvature_2;
    float theta_range_2;
    float sphere_center_2;
    float center_pos_2;
    float optical_property_2_refractive_index;;
    float optical_property_2_abbe_number;;
    float diameter_3;
    float radius_of_curvature_3;
    float theta_range_3;
    float sphere_center_3;
    float center_pos_3;
    float optical_property_3_refractive_index;;
    float optical_property_3_abbe_number;;
    float diameter_4;
    float center_pos_4;
    float optical_property_4_refractive_index;;
    float optical_property_4_abbe_number;;
    float diameter_5;
    float radius_of_curvature_5;
    float theta_range_5;
    float sphere_center_5;
    float center_pos_5;
    float optical_property_5_refractive_index;;
    float optical_property_5_abbe_number;;
    float radius_6;
    float center_pos_6;
    float optical_property_6_refractive_index;;
    float diameter_7;
    float radius_of_curvature_7;
    float theta_range_7;
    float sphere_center_7;
    float center_pos_7;
    float optical_property_7_refractive_index;;
    float optical_property_7_abbe_number;;
    float diameter_8;
    float center_pos_8;
    float optical_property_8_refractive_index;;
    float optical_property_8_abbe_number;;
    float diameter_9;
    float radius_of_curvature_9;
    float theta_range_9;
    float sphere_center_9;
    float center_pos_9;
    float optical_property_9_refractive_index;;
    float optical_property_9_abbe_number;;
    float diameter_10;
    float radius_of_curvature_10;
    float theta_range_10;
    float sphere_center_10;
    float center_pos_10;
    float optical_property_10_refractive_index;;
    float optical_property_10_abbe_number;;
    float diameter_11;
    float radius_of_curvature_11;
    float theta_range_11;
    float sphere_center_11;
    float center_pos_11;
    float optical_property_11_refractive_index;;
    float optical_property_11_abbe_number;;
};
ConstantBuffer<LensSystemData> lens_system_data;
    RWStructuredBuffer<RayInfo> ray_visualization_0;
    RWStructuredBuffer<RayInfo> ray_visualization_1;
    RWStructuredBuffer<RayInfo> ray_visualization_2;
    RWStructuredBuffer<RayInfo> ray_visualization_3;
    RWStructuredBuffer<RayInfo> ray_visualization_4;
    RWStructuredBuffer<RayInfo> ray_visualization_5;
    RWStructuredBuffer<RayInfo> ray_visualization_6;
    RWStructuredBuffer<RayInfo> ray_visualization_7;
    RWStructuredBuffer<RayInfo> ray_visualization_8;
    RWStructuredBuffer<RayInfo> ray_visualization_9;
    RWStructuredBuffer<RayInfo> ray_visualization_10;
    RWStructuredBuffer<RayInfo> ray_visualization_11;
RayInfo raygen(int2 pixel_id, inout float3 weight, inout uint seed)
{
    RayInfo ray;
    RayInfo next_ray;
    float t = 0;
    float2 film_pos = -((0.5f+float2(pixel_id)) / float2(lens_system_data.film_resolution)-0.5f) * lens_system_data.film_size;
    ray.Origin = float3(film_pos, -lens_system_data.film_distance);
    ray.TMin = 0;
    ray.TMax = 1000;
    next_ray = ray;;
    weight = 1;
    ray_visualization_0[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float2 seed2 = random_float2(seed);;
    float2 target_pos = sample_disk(seed2) * lens_system_data.diameter_1/2.0f;
    weight *= M_PI* lens_system_data.diameter_1/2.0f * lens_system_data.diameter_1/2.0f;
    float3 sampled_point_1 = float3(target_pos.x, target_pos.y, lens_system_data.center_pos_1);;
    ray.Direction = normalize(sampled_point_1 - ray.Origin);;
    weight*=ray.Direction.z;
    float relative_refractive_index_1 = get_relative_refractive_index(lens_system_data.optical_property_0_refractive_index, lens_system_data.optical_property_1_refractive_index);;
    next_ray =  intersect_sphere(ray, weight, t, lens_system_data.radius_of_curvature_1, lens_system_data.sphere_center_1, lens_system_data.theta_range_1, relative_refractive_index_1, lens_system_data.optical_property_1_abbe_number);;
    ray_visualization_1[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float relative_refractive_index_2 = get_relative_refractive_index(lens_system_data.optical_property_1_refractive_index, lens_system_data.optical_property_2_refractive_index);;
    next_ray =  intersect_sphere(ray, weight, t, lens_system_data.radius_of_curvature_2, lens_system_data.sphere_center_2, lens_system_data.theta_range_2, relative_refractive_index_2, lens_system_data.optical_property_2_abbe_number);;
    ray_visualization_2[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float relative_refractive_index_3 = get_relative_refractive_index(lens_system_data.optical_property_2_refractive_index, lens_system_data.optical_property_3_refractive_index);;
    next_ray =  intersect_sphere(ray, weight, t, lens_system_data.radius_of_curvature_3, lens_system_data.sphere_center_3, lens_system_data.theta_range_3, relative_refractive_index_3, lens_system_data.optical_property_3_abbe_number);;
    ray_visualization_3[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float relative_refractive_index_4 = get_relative_refractive_index(lens_system_data.optical_property_3_refractive_index, lens_system_data.optical_property_4_refractive_index);;
    next_ray = intersect_flat(ray, weight, t, lens_system_data.diameter_4, lens_system_data.center_pos_4, relative_refractive_index_4, lens_system_data.optical_property_4_abbe_number);;
    ray_visualization_4[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float relative_refractive_index_5 = get_relative_refractive_index(lens_system_data.optical_property_4_refractive_index, lens_system_data.optical_property_5_refractive_index);;
    next_ray =  intersect_sphere(ray, weight, t, lens_system_data.radius_of_curvature_5, lens_system_data.sphere_center_5, lens_system_data.theta_range_5, relative_refractive_index_5, lens_system_data.optical_property_5_abbe_number);;
    ray_visualization_5[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    next_ray = intersect_occluder(ray, weight, t, lens_system_data.radius_6, lens_system_data.center_pos_6);
    ray_visualization_6[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float relative_refractive_index_7 = get_relative_refractive_index(lens_system_data.optical_property_6_refractive_index, lens_system_data.optical_property_7_refractive_index);;
    next_ray =  intersect_sphere(ray, weight, t, lens_system_data.radius_of_curvature_7, lens_system_data.sphere_center_7, lens_system_data.theta_range_7, relative_refractive_index_7, lens_system_data.optical_property_7_abbe_number);;
    ray_visualization_7[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float relative_refractive_index_8 = get_relative_refractive_index(lens_system_data.optical_property_7_refractive_index, lens_system_data.optical_property_8_refractive_index);;
    next_ray = intersect_flat(ray, weight, t, lens_system_data.diameter_8, lens_system_data.center_pos_8, relative_refractive_index_8, lens_system_data.optical_property_8_abbe_number);;
    ray_visualization_8[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float relative_refractive_index_9 = get_relative_refractive_index(lens_system_data.optical_property_8_refractive_index, lens_system_data.optical_property_9_refractive_index);;
    next_ray =  intersect_sphere(ray, weight, t, lens_system_data.radius_of_curvature_9, lens_system_data.sphere_center_9, lens_system_data.theta_range_9, relative_refractive_index_9, lens_system_data.optical_property_9_abbe_number);;
    ray_visualization_9[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float relative_refractive_index_10 = get_relative_refractive_index(lens_system_data.optical_property_9_refractive_index, lens_system_data.optical_property_10_refractive_index);;
    next_ray =  intersect_sphere(ray, weight, t, lens_system_data.radius_of_curvature_10, lens_system_data.sphere_center_10, lens_system_data.theta_range_10, relative_refractive_index_10, lens_system_data.optical_property_10_abbe_number);;
    ray_visualization_10[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;
    float relative_refractive_index_11 = get_relative_refractive_index(lens_system_data.optical_property_10_refractive_index, lens_system_data.optical_property_11_refractive_index);;
    next_ray =  intersect_sphere(ray, weight, t, lens_system_data.radius_of_curvature_11, lens_system_data.sphere_center_11, lens_system_data.theta_range_11, relative_refractive_index_11, lens_system_data.optical_property_11_abbe_number);;
    ray_visualization_11[pixel_id.y * lens_system_data.film_resolution.x + pixel_id.x] = RayInfo(ray.Origin, 0, ray.Direction, t);;
    ray = next_ray;;

    return ray;
}