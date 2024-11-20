#define IMGUI_DEFINE_MATH_OPERATORS

#include "diff_optics/lens_system.hpp"

#include <fstream>
#include <memory>
#include <vector>

#include "dO_GUI.hpp"
#include "diff_optics/lens_system_compiler.hpp"
#include "imgui.h"
#include "optical_material.hpp"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

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

void NullPainter::control(DiffOpticsGUI* diff_optics_gui, LensLayer* get)
{
    // Slider control the center position

    ImGui::SliderFloat2(
        UniqueUIName("Center"), get->center_pos.data(), -40, 40);
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

void OccluderPainter::control(DiffOpticsGUI* diff_optics_gui, LensLayer* get)
{
    // float sliders
    auto occluder = dynamic_cast<Occluder*>(get);

    // Slider control the center position
    ImGui::SliderFloat2(
        UniqueUIName("Center"), occluder->center_pos.data(), -40, 40);

    ImGui::SliderFloat(UniqueUIName("Radius"), &occluder->radius, 0, 20);
}

void SphericalLens::update_info(float center_x, float center_y)
{
    theta_range = abs(asin(diameter / (2 * radius_of_curvature)));
    sphere_center = { center_x + radius_of_curvature, center_y };
}

SphericalLens::SphericalLens(float d, float roc, float center_x, float center_y)
    : LensLayer(center_x, center_y),
      diameter(d),
      radius_of_curvature(roc)
{
    update_info(center_x, center_y);
    painter = std::make_shared<SphericalLensPainter>();
}

BBox2D SphericalLensPainter::get_bounds(LensLayer* layer)
{
    auto film = dynamic_cast<SphericalLens*>(layer);

    auto roc = film->radius_of_curvature;
    auto diameter = film->diameter;
    auto center_pos = film->center_pos;
    auto right = film->sphere_center[0] - roc * cos(film->theta_range);

    return BBox2D{
        pxr::GfVec2f(center_pos[0], center_pos[1] - diameter / 2),
        pxr::GfVec2f(right, center_pos[1] + diameter / 2),
    };
}

void SphericalLensPainter::draw(
    DiffOpticsGUI* gui,
    LensLayer* layer,
    const pxr::GfMatrix3f& transform)
{
    auto center_pos = layer->center_pos;
    auto film = dynamic_cast<SphericalLens*>(layer);

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

void SphericalLensPainter::control(
    DiffOpticsGUI* diff_optics_gui,
    LensLayer* get)
{
    // float sliders
    auto film = dynamic_cast<SphericalLens*>(get);

    bool changed = false;

    changed |= ImGui::SliderFloat2(
        UniqueUIName("Center"), film->center_pos.data(), -40, 40);
    changed |=
        ImGui::SliderFloat(UniqueUIName("Diameter"), &film->diameter, 0, 20);
    changed |= ImGui::SliderFloat(
        UniqueUIName("Radius of Curvature"),
        &film->radius_of_curvature,
        -100,
        100);

    if (changed) {
        film->update_info(film->center_pos[0], film->center_pos[1]);
    }
}

FlatLens::FlatLens(float d, float center_x, float center_y)
    : LensLayer(center_x, center_y),
      diameter(d)
{
    painter = std::make_shared<FlatLensPainter>();
}

void FlatLens::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
    diameter = j["diameter"];
}

void FlatLens::EmitShader(
    int id,
    std::string& constant_buffer,
    std::string& execution,
    LensSystemCompiler* compiler)
{
    constant_buffer +=
        compiler->emit_line("float diameter_" + std::to_string(id), 1);
    constant_buffer +=
        compiler->emit_line("float center_pos_" + std::to_string(id), 1);
    // Optical Properties

    constant_buffer += compiler->emit_line(
        "float optical_property_" + std::to_string(id) + "_refractive_index;",
        1);
    constant_buffer += compiler->emit_line(
        "float optical_property_" + std::to_string(id) + "_abbe_number;", 1);

    if (id == 1) {
        assert(false);  // Not implemented
    }
    else {
        execution += compiler->emit_line(
            std::string("float relative_refractive_index_") +
            std::to_string(id) + " = get_relative_refractive_index(" +
            "lens_system_data.optical_property_" + std::to_string(id - 1) +
            "_refractive_index, " + "lens_system_data.optical_property_" +
            std::to_string(id) + "_refractive_index);");

        execution += compiler->emit_line(
            "next_ray = intersect_flat(ray, weight, t, "
            "lens_system_data.diameter_" +
            std::to_string(id) + ", lens_system_data.center_pos_" +
            std::to_string(id) + ", " + "relative_refractive_index_" +
            std::to_string(id) + ", " + "lens_system_data.optical_property_" +
            std::to_string(id) + "_abbe_number);");
    }
}

void FlatLens::fill_block_data(float* ptr)
{
    ptr[0] = diameter;
    ptr[1] = center_pos[0];
    ptr[2] = optical_property.refractive_index;
    ptr[3] = optical_property.abbe_number;
}

BBox2D FlatLensPainter::get_bounds(LensLayer* layer)
{
    auto film = dynamic_cast<FlatLens*>(layer);
    auto diameter = film->diameter;
    auto center_pos = film->center_pos;
    return BBox2D{
        pxr::GfVec2f(center_pos[0], center_pos[1] - diameter / 2),
        pxr::GfVec2f(center_pos[0] + 1, center_pos[1] + diameter / 2),
    };
}

void FlatLensPainter::draw(
    DiffOpticsGUI* gui,
    LensLayer* layer,
    const pxr::GfMatrix3f& transform)
{
    auto film = dynamic_cast<FlatLens*>(layer);

    auto center_pos = film->center_pos;
    auto diameter = film->diameter;

    auto ep1 = pxr::GfVec2f(center_pos[0], center_pos[1] + diameter / 2);
    auto ep2 = pxr::GfVec2f(center_pos[0], center_pos[1] - diameter / 2);

    auto tep1 = transform * pxr::GfVec3f(ep1[0], ep1[1], 1);
    auto tep2 = transform * pxr::GfVec3f(ep2[0], ep2[1], 1);

    gui->DrawLine(ImVec2(tep1[0], tep1[1]), ImVec2(tep2[0], tep2[1]));
}

void FlatLensPainter::control(DiffOpticsGUI* diff_optics_gui, LensLayer* get)
{
    // float sliders
    auto film = dynamic_cast<FlatLens*>(get);

    // Slider control the center position
    ImGui::SliderFloat2(
        UniqueUIName("Center"), film->center_pos.data(), -40, 40);

    ImGui::SliderFloat(UniqueUIName("Diameter"), &film->diameter, 0, 20);
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
                     canvas_size[1] / (bound.max[1] - bound.min[1])) *
                 0.85f;

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

void LensSystemGUI::control(DiffOpticsGUI* diff_optics_gui)
{
    // For each lens, give a imgui subgroup to control the lens

    for (auto&& lens_layer : lens_system->lenses) {
        if (ImGui::TreeNode(lens_layer->painter->UniqueUIName("Lens Layer"))) {
            lens_layer->painter->control(diff_optics_gui, lens_layer.get());
            ImGui::TreePop();
        }
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

void NullLayer::EmitShader(
    int id,
    std::string& constant_buffer,
    std::string& execution,
    LensSystemCompiler* compiler)
{
    constant_buffer += compiler->emit_line(
        "float optical_property_" + std::to_string(id) + "_refractive_index;",
        1);
}

void NullLayer::fill_block_data(float* ptr)
{
    ptr[0] = optical_property.refractive_index;
}

void Occluder::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
    radius = j["diameter"].get<float>() / 2.0f;
}

void SphericalLens::deserialize(const nlohmann::json& j)
{
    LensLayer::deserialize(j);
    diameter = j["diameter"];
    radius_of_curvature = j["roc"];
    theta_range = abs(atan(diameter / (2 * radius_of_curvature)));
    sphere_center = { center_pos[0] + radius_of_curvature, center_pos[1] };
    // if (j.contains("additional_params")) {
    //     high_order_polynomial_coefficients =
    //         j["additional_params"].get<std::vector<float>>();
    // }
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
        else if (item["type"] == "S" && item["roc"].get<float>() != 0) {
            layer = std::make_shared<SphericalLens>(
                item["diameter"].get<float>(),
                item["roc"],
                accumulated_distance,
                0.0f);
        }
        else if (item["type"] == "S" && item["roc"].get<float>() == 0) {
            layer = std::make_shared<FlatLens>(
                item["diameter"].get<float>(), accumulated_distance, 0.0f);
        }

        layer->deserialize(item);
        add_lens(layer);
    }
}

void LensSystem::deserialize(const std::filesystem::path& path)
{
    std::ifstream json_file(path);
    std::string json(
        (std::istreambuf_iterator<char>(json_file)),
        std::istreambuf_iterator<char>());
    deserialize(json);
}

static const std::string sphere_raygen_template = R"(
    RayInfo
)";

void Occluder::EmitShader(
    int id,
    std::string& constant_buffer,
    std::string& execution,
    LensSystemCompiler* compiler)
{
    constant_buffer +=
        compiler->emit_line("float radius_" + std::to_string(id), 1);
    constant_buffer +=
        compiler->emit_line("float center_pos_" + std::to_string(id), 1);
    constant_buffer += compiler->emit_line(
        "float optical_property_" + std::to_string(id) + "_refractive_index;",
        1);

    if (id == 1) {
        throw std::runtime_error("Not implemented");
    }
    else {
        execution += compiler->emit_line(
            std::string("next_ray = intersect_occluder(ray, weight, t, ") +
            "lens_system_data.radius_" + std::to_string(id) + ", " +
            "lens_system_data.center_pos_" + std::to_string(id) + ")");
    }
}

void Occluder::fill_block_data(float* ptr)
{
    ptr[0] = radius;
    ptr[1] = center_pos[0];
    ptr[2] = optical_property.refractive_index;
}

void SphericalLens::EmitShader(
    int id,
    std::string& constant_buffer,
    std::string& execution,
    LensSystemCompiler* compiler)
{
    constant_buffer +=
        compiler->emit_line("float diameter_" + std::to_string(id), 1);
    constant_buffer += compiler->emit_line(
        "float radius_of_curvature_" + std::to_string(id), 1);

    constant_buffer +=
        compiler->emit_line("float theta_range_" + std::to_string(id), 1);
    constant_buffer +=
        compiler->emit_line("float sphere_center_" + std::to_string(id), 1);

    // center_pos
    constant_buffer +=
        compiler->emit_line("float center_pos_" + std::to_string(id), 1);

    //// Also optical parameters

    constant_buffer += compiler->emit_line(
        "float optical_property_" + std::to_string(id) + "_refractive_index;",
        1);
    constant_buffer += compiler->emit_line(
        "float optical_property_" + std::to_string(id) + "_abbe_number;", 1);

    if (id == 1) {
        // Sample the target points with the first lens radius, and then
        // calculate the direction based on it.

        // suppose we can use random_float2
        execution += compiler->emit_line("float2 seed2 = random_float2(seed);");
        execution += compiler->emit_line(
            std::string("float2 target_pos = sample_disk(seed2) * ") +
            "lens_system_data.diameter_" + std::to_string(id) + "/2.0f");
        execution += compiler->emit_line(
            "float3 sampled_point_" + std::to_string(id) +
            " = float3(target_pos.x, target_pos.y, " +
            "lens_system_data.center_pos_" + std::to_string(id) + ");");

        execution += compiler->emit_line(
            "ray.Direction = normalize(sampled_point_" + std::to_string(id) +
            " - ray.Origin);");
        execution += compiler->emit_line("weight*=ray.Direction.z");
    }

    execution += compiler->emit_line(
        std::string("float relative_refractive_index_") + std::to_string(id) +
        " = get_relative_refractive_index(" +
        "lens_system_data.optical_property_" + std::to_string(id - 1) +
        "_refractive_index, " + "lens_system_data.optical_property_" +
        std::to_string(id) + "_refractive_index);");

    execution += compiler->emit_line(
        std::string("next_ray =  intersect_sphere(ray, weight, t, ") +
        "lens_system_data.radius_of_curvature_" + std::to_string(id) + ", " +
        "lens_system_data.sphere_center_" + std::to_string(id) + ", " +
        "lens_system_data.theta_range_" + std::to_string(id) + ", " +
        "relative_refractive_index_" + std::to_string(id) + ", " +
        +"lens_system_data.optical_property_" + std::to_string(id) +
        "_abbe_number);");
}

void SphericalLens::fill_block_data(float* ptr)
{
    ptr[0] = diameter;
    ptr[1] = radius_of_curvature;
    ptr[2] = theta_range;
    ptr[3] = sphere_center[0];
    ptr[4] = center_pos[0];
    ptr[5] = optical_property.refractive_index;
    ptr[6] = optical_property.abbe_number;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
