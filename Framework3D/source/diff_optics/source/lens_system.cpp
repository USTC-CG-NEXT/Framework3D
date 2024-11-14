#define IMGUI_DEFINE_MATH_OPERATORS

#include "diff_optics/lens_system.hpp"

#include <memory>
#include <vector>

#include "dO_GUI.hpp"
#include "imgui.h"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

static unsigned indent = 0;

static std::string indent_str(unsigned n)
{
    return std::string(n, ' ');
}

OpticalProperty get_optical_property(const std::string& name)
{
    std::string upper_name = name;
    std::transform(
        upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);

    if (upper_name == "VACUUM") {
        return OpticalProperty{ 1.0, std::numeric_limits<float>::infinity() };
    }
    else if (upper_name == "AIR") {
        return OpticalProperty{ 1.000293,
                                std::numeric_limits<float>::infinity() };
    }
    else if (upper_name == "OCCLUDER") {
        return OpticalProperty{ 1.0, std::numeric_limits<float>::infinity() };
    }
    else if (upper_name == "F2") {
        return OpticalProperty{ 1.620, 36.37 };
    }
    else if (upper_name == "F15") {
        return OpticalProperty{ 1.60570, 37.831 };
    }
    else if (upper_name == "UVFS") {
        return OpticalProperty{ 1.458, 67.82 };
    }
    else if (upper_name == "BK10") {
        return OpticalProperty{ 1.49780, 66.954 };
    }
    else if (upper_name == "N-BAF10") {
        return OpticalProperty{ 1.67003, 47.11 };
    }
    else if (upper_name == "N-BK7") {
        return OpticalProperty{ 1.51680, 64.17 };
    }
    else if (upper_name == "N-SF1") {
        return OpticalProperty{ 1.71736, 29.62 };
    }
    else if (upper_name == "N-SF2") {
        return OpticalProperty{ 1.64769, 33.82 };
    }
    else if (upper_name == "N-SF4") {
        return OpticalProperty{ 1.75513, 27.38 };
    }
    else if (upper_name == "N-SF5") {
        return OpticalProperty{ 1.67271, 32.25 };
    }
    else if (upper_name == "N-SF6") {
        return OpticalProperty{ 1.80518, 25.36 };
    }
    else if (upper_name == "N-SF6HT") {
        return OpticalProperty{ 1.80518, 25.36 };
    }
    else if (upper_name == "N-SF8") {
        return OpticalProperty{ 1.68894, 31.31 };
    }
    else if (upper_name == "N-SF10") {
        return OpticalProperty{ 1.72828, 28.53 };
    }
    else if (upper_name == "N-SF11") {
        return OpticalProperty{ 1.78472, 25.68 };
    }
    else if (upper_name == "SF1") {
        return OpticalProperty{ 1.71736, 29.51 };
    }
    else if (upper_name == "SF2") {
        return OpticalProperty{ 1.64769, 33.85 };
    }
    else if (upper_name == "SF4") {
        return OpticalProperty{ 1.75520, 27.58 };
    }
    else if (upper_name == "SF5") {
        return OpticalProperty{ 1.67270, 32.21 };
    }
    else if (upper_name == "SF6") {
        return OpticalProperty{ 1.80518, 25.43 };
    }
    else if (upper_name == "SF18") {
        return OpticalProperty{ 1.72150, 29.245 };
    }
    else if (upper_name == "BAF10") {
        return OpticalProperty{ 1.67, 47.05 };
    }
    else if (upper_name == "SK1") {
        return OpticalProperty{ 1.61030, 56.712 };
    }
    else if (upper_name == "SK16") {
        return OpticalProperty{ 1.62040, 60.306 };
    }
    else if (upper_name == "SSK4") {
        return OpticalProperty{ 1.61770, 55.116 };
    }
    else if (upper_name == "B270") {
        return OpticalProperty{ 1.52290, 58.50 };
    }
    else if (upper_name == "S-NPH1") {
        return OpticalProperty{ 1.8078, 22.76 };
    }
    else if (upper_name == "D-K59") {
        return OpticalProperty{ 1.5175, 63.50 };
    }
    else if (upper_name == "FLINT") {
        return OpticalProperty{ 1.6200, 36.37 };
    }
    else if (upper_name == "PMMA") {
        return OpticalProperty{ 1.491756, 58.00 };
    }
    else if (upper_name == "POLYCARB") {
        return OpticalProperty{ 1.585470, 30.00 };
    }
    else {
        return OpticalProperty{ 1.0, 0.0 };
    }
}

BBox2D::BBox2D() : min(1e9, 1e9), max(-1e9, -1e9)
{
}

BBox2D::BBox2D(pxr::GfVec2f min, pxr::GfVec2f max) : min(min), max(max)
{
}

pxr::GfVec2f BBox2D::center() const
{
    return (min + max) / 2;
}

BBox2D BBox2D::operator+(const BBox2D& b) const
{
    // Bounding box merge
    return BBox2D{
        pxr::GfVec2f(std::min(min[0], b.min[0]), std::min(min[1], b.min[1])),
        pxr::GfVec2f(std::max(max[0], b.max[0]), std::max(max[1], b.max[1])),
    };
}

BBox2D& BBox2D::operator+=(const BBox2D& b)
{
    min[0] = std::min(min[0], b.min[0]);
    min[1] = std::min(min[1], b.min[1]);
    max[0] = std::max(max[0], b.max[0]);
    max[1] = std::max(max[1], b.max[1]);
    return *this;
}

LensLayer::LensLayer(float center_x, float center_y)
    : center_pos(center_x, center_y)
{
}

LensLayer::~LensLayer() = default;

void LensLayer::set_axis(float axis_pos)
{
    center_pos[1] = axis_pos;
}

void LensLayer::set_pos(float x)
{
    center_pos[0] = x;
}

NullLayer::NullLayer(float center_x, float center_y)
    : LensLayer(center_x, center_y)
{
    painter = std::make_shared<NullPainter>();
}

Occluder::Occluder(float radius, float x, float y)
    : radius(radius),
      LensLayer(x, y)
{
    painter = std::make_shared<OccluderPainter>();
}

BBox2D OccluderPainter::get_bounds(LensLayer* layer)
{
    auto pupil = dynamic_cast<Occluder*>(layer);
    auto radius = pupil->radius;
    auto center_pos = pupil->center_pos;
    return BBox2D{
        pxr::GfVec2f(center_pos[0], center_pos[1] - radius - 1),
        pxr::GfVec2f(center_pos[0] + 1, center_pos[1] + radius + 1),
    };
}

void OccluderPainter::draw(
    DiffOpticsGUI* gui,
    LensLayer* layer,
    const pxr::GfMatrix3f& xform)
{
    auto pupil = dynamic_cast<Occluder*>(layer);

    pxr::GfVec2f ep1 = { pupil->center_pos[0],
                         pupil->center_pos[1] + pupil->radius };
    pxr::GfVec2f ep2 = { pupil->center_pos[0],
                         pupil->center_pos[1] + pupil->radius + 1 };
    pxr::GfVec2f ep3 = { pupil->center_pos[0],
                         pupil->center_pos[1] + -pupil->radius };
    pxr::GfVec2f ep4 = { pupil->center_pos[0],
                         pupil->center_pos[1] + -pupil->radius - 1 };

    pxr::GfVec3f tep1 = xform * pxr::GfVec3f(ep1[0], ep1[1], 1);
    pxr::GfVec2f tep1_2d = pxr::GfVec2f(tep1[0], tep1[1]);

    pxr::GfVec3f tep2 = xform * pxr::GfVec3f(ep2[0], ep2[1], 1);
    pxr::GfVec2f tep2_2d = pxr::GfVec2f(tep2[0], tep2[1]);

    pxr::GfVec3f tep3 = xform * pxr::GfVec3f(ep3[0], ep3[1], 1);
    pxr::GfVec2f tep3_2d = pxr::GfVec2f(tep3[0], tep3[1]);

    pxr::GfVec3f tep4 = xform * pxr::GfVec3f(ep4[0], ep4[1], 1);
    pxr::GfVec2f tep4_2d = pxr::GfVec2f(tep4[0], tep4[1]);

    gui->DrawLine(
        ImVec2(tep1_2d[0], tep1_2d[1]), ImVec2(tep2_2d[0], tep2_2d[1]));
    gui->DrawLine(
        ImVec2(tep3_2d[0], tep3_2d[1]), ImVec2(tep4_2d[0], tep4_2d[1]));
}

LensFilm::LensFilm(float d, float roc, float center_x, float center_y)
    : LensLayer(center_x, center_y),
      diameter(d),
      radius_of_curvature(roc),
      optical_property()
{
    theta_range = atan(diameter / (2 * radius_of_curvature));
    sphere_center = { center_x + radius_of_curvature, center_y };
    painter = std::make_shared<LensFilmPainter>();
}

BBox2D LensFilmPainter::get_bounds(LensLayer* layer)
{
    auto film = dynamic_cast<LensFilm*>(layer);

    auto roc = film->radius_of_curvature;
    auto diameter = film->diameter;
    auto center_pos = film->center_pos;
    auto right = film->sphere_center[0] - roc * cos(film->theta_range);

    return BBox2D{
        pxr::GfVec2f(center_pos[0], center_pos[1] - diameter),
        pxr::GfVec2f(right, center_pos[1] + diameter),
    };
}

void LensFilmPainter::draw(
    DiffOpticsGUI* gui,
    LensLayer* layer,
    const pxr::GfMatrix3f& transform)
{
    auto center_pos = layer->center_pos;
    auto film = dynamic_cast<LensFilm*>(layer);

    auto transformed_sphere_center =
        transform *
        pxr::GfVec3f(film->sphere_center[0], film->sphere_center[1], 1);

    float theta_min = -film->theta_range + M_PI;
    float theta_max = film->theta_range + M_PI;
    if (film->radius_of_curvature < 0) {
        theta_min = -film->theta_range;
        theta_max = film->theta_range;
    }
    gui->DrawArc(
        ImVec2(transformed_sphere_center[0], transformed_sphere_center[1]),
        abs(film->radius_of_curvature) * transform[0][0],
        theta_min,
        theta_max);
}

LensSystem::LensSystem() : gui(std::make_unique<LensSystemGUI>(this))
{
}

void LensSystem::add_lens(std::shared_ptr<LensLayer> lens)
{
    lenses.push_back(lens);
}

void LensSystemGUI::set_canvas_size(float x, float y)
{
    canvas_size = pxr::GfVec2f(x, y);
}

void LensSystemGUI::draw(DiffOpticsGUI* gui) const
{
    // First compute the scale based on collected lens information
    auto bound = BBox2D();

    for (auto& lens : lens_system->lenses) {
        auto lens_bounds = lens->painter->get_bounds(lens.get());
        bound += lens_bounds;
    }

    auto bound_center = bound.center();
    auto canvas_center = pxr::GfVec2f(canvas_size[0] / 2, canvas_size[1] / 2);

    auto scale = std::min(
        canvas_size[0] / (bound.max[0] - bound.min[0]),
        canvas_size[1] / (bound.max[1] - bound.min[1]));

    // create transform such that send center of bound to center of canvas, and
    // scale the bound to fit the canvas

    pxr::GfMatrix3f transform =
        pxr::GfMatrix3f{
            scale,
            0,
            0,
            0,
            scale,
            0,
            canvas_center[0] - bound_center[0] * scale,
            canvas_center[1] - bound_center[1] * scale,
            1,
        }
            .GetTranspose();

    for (auto& lens : lens_system->lenses) {
        lens->painter->draw(gui, lens.get(), transform);
    }
}

void LensLayer::deserialize(const nlohmann::json& j)
{
    optical_property = get_optical_property(j["material"]);
}

void NullLayer::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
}

void Occluder::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
    radius = j["diameter"].get<float>() / 2.0f;
}

void LensFilm::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
    diameter = j["diameter"];
    radius_of_curvature = j["roc"];
    theta_range = atan(diameter / (2 * radius_of_curvature));
    sphere_center = { center_pos[0] + radius_of_curvature, center_pos[1] };
    if (j.contains("additional_params")) {
        high_order_polynomial_coefficients =
            j["additional_params"].get<std::vector<float>>();
    }
}

void LensSystem::deserialize(const std::string& json)
{
    nlohmann::json j = nlohmann::json::parse(json);
    float accumulated_distance = 0.0f;
    for (const auto& item : j["data"]) {
        std::shared_ptr<LensLayer> layer;
        accumulated_distance += item["distance"].get<float>();
        if (item["type"] == "O") {
            layer = std::make_shared<NullLayer>(accumulated_distance, 0.0f);
        }
        else if (item["type"] == "A") {
            layer = std::make_shared<Occluder>(
                item["diameter"].get<float>() / 2.0f,
                accumulated_distance,
                0.0f);
        }
        else if (item["type"] == "S") {
            layer = std::make_shared<LensFilm>(
                item["diameter"].get<float>() / 2.0f,
                item["roc"],
                accumulated_distance,
                0.0f);
        }

        layer->deserialize(item);
        add_lens(layer);
    }
}
static const std::string sphere_raygen = R"(

RayDesc intersect_sphere(
    RayDesc ray,
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
        return RayDesc(ray_pos, 0, ray_dir, 0.0);
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
        return RayDesc(ray_pos, 0, ray_dir, 0.0);
    }

    return RayDesc(intersection_pos, 0, refracted_dir, 1000.f);
}

)";

static const std::string sphere_raygen_template = R"(
    RayDesc
)";

void Occluder::EmitShader(
    int id,
    std::string& constant_buffer,
    std::string& execution)
{
    constant_buffer += indent_str(indent) + "float occluder_radius_" +
                       std::to_string(id) + ";\n";
}

void LensFilm::EmitShader(
    int id,
    std::string& constant_buffer,
    std::string& execution)
{
    constant_buffer += indent_str(indent) + "float lens_diameter_" +
                       std::to_string(id) + ";\n";
    constant_buffer += indent_str(indent) + "float lens_radius_of_curvature_" +
                       std::to_string(id) + ";\n";

    constant_buffer += indent_str(indent) + "float lens_theta_range_" +
                       std::to_string(id) + ";\n";
    constant_buffer += indent_str(indent) + "float lens_sphere_center_" +
                       std::to_string(id) + ";\n";

    //// Also optical parameters

    constant_buffer += indent_str(indent) + "float lens_optical_property_" +
                       std::to_string(id) + "_refractive_index;\n";
    constant_buffer += indent_str(indent) + "float lens_optical_property_" +
                       std::to_string(id) + "_abbe_number;\n";

    if (id == 1) {
        // Sample the target points with the first lens radius, and then
        // calculate the direction based on it.

        execution += indent_str(indent) + "float3 sphere_center_" +
                     std::to_string(id) + " = float3(0, 0, " +
                     "lens_system_data.lens_sphere_center_" +
                     std::to_string(id) + ");\n";
        // suppose we can use random_float2
        execution +=
            indent_str(indent) + "float2 seed2 = random_float2(seed);\n";
        execution +=
            indent_str(indent) + "float2 target_pos = sample_disk(seed2) * " +
            "lens_system_data.lens_diameter_" + std::to_string(id) + ";\n";
    }
    else {
        execution +=
            indent_str(indent) + "ray =  intersect_sphere(ray, weight, " +
            "lens_system_data.lens_diameter_" + std::to_string(id) + ", " +
            "lens_system_data.lens_sphere_center_" + std::to_string(id) + ", " +
            "lens_system_data.lens_theta_range_" + std::to_string(id) + ", " +
            "lens_system_data.lens_optical_property_" + std::to_string(id) +
            "_refractive_index, " + "lens_system_data.lens_optical_property_" +
            std::to_string(id) + "_abbe_number);\n";
    }
}

std::string LensSystem::gen_slang_shader()
{
    std::string header = R"(
#include "utils/random.slangh"
import Utils.Math.MathHelpers;
)";
    std::string functions = sphere_raygen + "\n";

    indent += 4;

    std::string const_buffer = "struct LensSystemData\n{\n";
    const_buffer += indent_str(indent) + "float2 film_size;\n";
    const_buffer += indent_str(indent) + "int2 film_resolution;\n";
    const_buffer += indent_str(indent) + "float film_distance;\n";

    std::string raygen_shader =
        "RayDesc raygen(int2 pixel_id, inout float weight, inout uint seed)\n{";

    int id = 0;

    raygen_shader += "\n";
    raygen_shader += indent_str(indent) + "RayDesc ray;\n";

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

    for (auto lens_layer : lenses) {
        lens_layer->EmitShader(id, const_buffer, raygen_shader);
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
