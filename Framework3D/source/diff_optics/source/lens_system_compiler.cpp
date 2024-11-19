#include "diff_optics/lens_system_compiler.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
const std::string LensSystemCompiler::sphere_intersection = R"(

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

    // Calculate the refracted direction using Snell's law
    float eta = refractive_index;
    float3 refracted_dir = normalize( refract(ray_dir, normal, eta));
    // Calculate the angle of the intersection point to the normalized sphere
    // center
    float3 normalized_sphere_center = normalize(-sphere_center);
    float angle = abs(acos(dot(normal, normalized_sphere_center)));

    // Check if the angle is within the allowed range
    if (angle > alpha_range) {
        // No valid intersection within the alpha range
        //return RayInfo(ray_pos, 0, ray_dir, 0.0);
    }

    return RayInfo(intersection_pos, 0, refracted_dir, 1000.f);
}

)";

const std::string LensSystemCompiler::flat_intersection = R"(
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
)";

const std::string LensSystemCompiler::occluder_intersection = R"(
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
)";

const std::string get_relative_refractive_index = R"(
float get_relative_refractive_index(float refract_id_last, float refract_id_this)
{
	return refract_id_this / refract_id_last;
}
)";

std::string LensSystemCompiler::emit_line(
    const std::string& line,
    unsigned cb_size_occupied)
{
    cb_size += cb_size_occupied;
    return indent_str(indent) + line + ";\n";
}

std::tuple<std::string, CompiledDataBlock> LensSystemCompiler::compile(
    LensSystem* lens_system,
    bool require_ray_visualization)
{
    cb_size = 0;

    std::string header = R"(
#include "utils/random.slangh"
#include "utils/ray.h"
import Utils.Math.MathHelpers;
)";
    std::string functions = sphere_intersection + "\n";
    functions += flat_intersection + "\n";
    functions += occluder_intersection + "\n";
    functions += get_relative_refractive_index + "\n";
    std::string const_buffer;

    indent += 4;

    const_buffer += "struct LensSystemData\n{\n";
    const_buffer += emit_line("float2 film_size;", 2);
    const_buffer += emit_line("int2 film_resolution;", 2);
    const_buffer += emit_line("float film_distance;", 1);

    std::string raygen_shader =
        "RayInfo raygen(int2 pixel_id, inout float weight, inout uint "
        "seed)\n{";

    int id = 0;

    raygen_shader += "\n";
    raygen_shader += indent_str(indent) + "RayInfo ray;\n";
    raygen_shader += indent_str(indent) + "RayInfo next_ray;\n";
    raygen_shader += emit_line("float t = 0");

    // Sample origin on the film
    raygen_shader += indent_str(indent) +
                     "float2 film_pos = -((0.5f+float2(pixel_id)) / "
                     "float2(lens_system_data.film_resolution)-0.5f) * "
                     "lens_system_data.film_size;\n";

    raygen_shader += indent_str(indent) +
                     "ray.Origin = float3(film_pos, "
                     "-lens_system_data.film_distance);\n";

    // TMin and TMax

    raygen_shader += indent_str(indent) + "ray.TMin = 0;\n";
    raygen_shader += indent_str(indent) + "ray.TMax = 1000;\n";

    raygen_shader += emit_line("next_ray = ray;");

    raygen_shader += indent_str(indent) + "float3 weight = 1;\n";

    CompiledDataBlock block;

    for (auto lens_layer : lens_system->lenses) {
        block.parameter_offsets[id] = cb_size;
        lens_layer->EmitShader(id, const_buffer, raygen_shader, this);

        if (require_ray_visualization) {
            raygen_shader += emit_line(
                "ray_visualization_" + std::to_string(id) +
                "[pixel_id.y * lens_system_data.film_resolution.x + "
                "pixel_id.x] "
                "= RayInfo(ray.Origin, 0, ray.Direction, t);");
        }
        raygen_shader += emit_line("ray = next_ray;");
        id++;
    }

    block.parameters.resize(cb_size);
    block.cb_size = cb_size;

    for (int i = 0; i < lens_system->lenses.size(); ++i) {
        auto lens = lens_system->lenses[i];

        auto offset = block.parameter_offsets[i];

        lens->fill_block_data(block.parameters.data() + offset);
    }

    const_buffer += "};\n";
    const_buffer += "ConstantBuffer<LensSystemData> lens_system_data;\n";

    if (require_ray_visualization) {
        for (size_t i = 0; i < lens_system->lenses.size(); i++) {
            const_buffer += emit_line(
                "RWStructuredBuffer<RayInfo> ray_visualization_" +
                std::to_string(i));
        }
    }

    raygen_shader += "\n";
    raygen_shader += indent_str(indent) + "return ray;\n";
    raygen_shader += "}";

    indent -= 4;

    auto final_shader = header + functions + const_buffer + raygen_shader;

    return std::make_tuple(final_shader, block);
}

void LensSystemCompiler::fill_block_data(
    LensSystem* lens_system,
    CompiledDataBlock& data_block)
{
    for (int i = 0; i < lens_system->lenses.size(); ++i) {
        auto lens = lens_system->lenses[i];

        auto offset = data_block.parameter_offsets[i];

        lens->fill_block_data(data_block.parameters.data() + offset);
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE