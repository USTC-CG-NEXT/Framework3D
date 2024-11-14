#include "diff_optics/lens_system_compiler.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
const std::string LensSystemCompiler::sphere_intersection = R"(

RayInfo intersect_sphere(
    RayInfo ray,
    inout float3 weight,
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
        return RayInfo(ray_pos, 0, ray_dir, 0.0);
    }

    // Calculate the distance to the intersection points
    float t1 = (-b - sqrt(discriminant)) / (2.0 * a);
    float t2 = (-b + sqrt(discriminant)) / (2.0 * a);

    // Choose the closest positive intersection
    float t = (t1 > 0.0) ? t1 : t2;

    // Calculate the intersection position
    float3 intersection_pos = ray_pos + t * ray_dir;

    // Calculate the normal at the intersection point
    float3 normal = normalize(intersection_pos - sphere_center);

    // Calculate the refracted direction using Snell's law
    float eta = 1.0 / refractive_index;
    float cosi = dot(-ray_dir, normal);
    float k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    float3 refracted_dir =
        (k < 0.0) ? float3(0.0, 0.0, 0.0)
                  : eta * ray_dir + (eta * cosi - sqrt(k)) * normal;
    // Calculate the angle of the intersection point to the normalized sphere
    // center
    float3 normalized_sphere_center = normalize(-sphere_center);
    float angle = acos(dot(normal, normalized_sphere_center));

    // Check if the angle is within the allowed range
    if (angle > alpha_range) {
        // No valid intersection within the alpha range
        return RayInfo(ray_pos, 0, ray_dir, 0.0);
    }

    return RayInfo(intersection_pos, 0, refracted_dir, 1000.f);
}

)";

const std::string LensSystemCompiler::flat_intersection = R"(
RayInfo intersect_flat(
    RayInfo ray,
    inout float3 weight,
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
    float t = dot(plane_center - ray_pos, float3(0, 0, 1)) / dot(ray_dir, float3(0, 0, 1));

    // Calculate the intersection position
    float3 intersection_pos = ray_pos + t * ray_dir;

    // Calculate the normal at the intersection point
    float3 normal = float3(0, 0, 1);

    // Calculate the refracted direction using Snell's law
    float eta = 1.0 / refractive_index;
    float cosi = dot(-ray_dir, normal);
    float k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    float3 refracted_dir =
        (k < 0.0) ? float3(0.0, 0.0, 0.0)
                  : eta * ray_dir + (eta * cosi - sqrt(k)) * normal;

    return RayInfo(intersection_pos, 0, refracted_dir, 1000.f);
}
)";

unsigned LensSystemCompiler::indent = 0;

std::string LensSystemCompiler::compile(LensSystem* lens_system)
{
    std::string header = R"(
#include "utils/random.slangh"
#include "utils/ray.h"
import Utils.Math.MathHelpers;
)";
    std::string functions = sphere_intersection + "\n";
    functions += flat_intersection + "\n";

    indent += 4;

    std::string const_buffer = "struct LensSystemData\n{\n";
    const_buffer += indent_str(indent) + "float2 film_size;\n";
    const_buffer += indent_str(indent) + "int2 film_resolution;\n";
    const_buffer += indent_str(indent) + "float film_distance;\n";

    std::string raygen_shader =
        "RayInfo raygen(int2 pixel_id, inout float weight, inout uint "
        "seed)\n{";

    int id = 0;

    raygen_shader += "\n";
    raygen_shader += indent_str(indent) + "RayInfo ray;\n";

    // Sample origin on the film
    raygen_shader += indent_str(indent) +
                     "float2 film_pos = (0.5f+float2(pixel_id)) / "
                     "float2(lens_system_data.film_resolution);\n";

    raygen_shader += indent_str(indent) +
                     "ray.Origin = float3(film_pos, "
                     "-lens_system_data.film_distance);\n";

    // TMin and TMax

    raygen_shader += indent_str(indent) + "ray.TMin = 0;\n";
    raygen_shader += indent_str(indent) + "ray.TMax = 1000;\n";

    raygen_shader += indent_str(indent) + "float3 weight = 1;\n";

    cb_size = 0;

    for (auto lens_layer : lens_system->lenses) {
        lens_layer->EmitShader(id, const_buffer, raygen_shader);
        cb_size += lens_layer->get_cb_size();
        id++;
    }

    const_buffer += "};\n";
    const_buffer += "ConstantBuffer<LensSystemData> lens_system_data;\n";

    raygen_shader += "\n";
    raygen_shader += indent_str(indent) + "return ray;\n";
    raygen_shader += "}";

    indent -= 4;

    return header + functions + const_buffer + raygen_shader;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE