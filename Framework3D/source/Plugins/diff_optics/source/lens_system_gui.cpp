#include "lens_system_gui.hpp"

#include "dO_GUI.hpp"

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

bool NullPainter::control(DiffOpticsGUI* diff_optics_gui, LensLayer* get)
{
    // Slider control the center position

    bool changed = false;
    if (ImGui::TreeNode(UniqueUIName("Null"))) {
        changed |= ImGui::SliderFloat2(
            UniqueUIName("Center"), get->center_pos.data(), -40, 40);
        ImGui::TreePop();
    }

    return changed;
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

bool OccluderPainter::control(DiffOpticsGUI* diff_optics_gui, LensLayer* get)
{
    // float sliders
    bool changed = false;

    if (ImGui::TreeNode(UniqueUIName("Occluder"))) {
        auto occluder = dynamic_cast<Occluder*>(get);

        // Slider control the center position
        changed |= ImGui::SliderFloat2(
            UniqueUIName("Center"), occluder->center_pos.data(), -40, 40);
        // Slider control the radius
        changed |= ImGui::SliderFloat(
            UniqueUIName("Radius"), &occluder->radius, 0, 20);
        ImGui::TreePop();
    }

    return changed;
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

bool SphericalLensPainter::control(
    DiffOpticsGUI* diff_optics_gui,
    LensLayer* get)
{
    // float sliders
    bool changed = false;

    if (ImGui::TreeNode(UniqueUIName("Spherical Lens"))) {
        auto film = dynamic_cast<SphericalLens*>(get);
        // Slider control the center position
        changed |= ImGui::SliderFloat2(
            UniqueUIName("Center"), film->center_pos.data(), -40, 40);
        // Slider control the diameter
        changed |= ImGui::SliderFloat(
            UniqueUIName("Diameter"), &film->diameter, 0, 20);
        // Slider control the radius of curvature
        changed |= ImGui::SliderFloat(
            UniqueUIName("Radius of Curvature"),
            &film->radius_of_curvature,
            -100,
            100);
        // Slider control the theta range
        changed |= ImGui::SliderFloat(
            UniqueUIName("Theta Range"), &film->theta_range, 0, M_PI);
        ImGui::TreePop();

        if (changed) {
            film->update_info(film->center_pos[0], film->center_pos[1]);
        }
    }

    return changed;
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

bool FlatLensPainter::control(DiffOpticsGUI* diff_optics_gui, LensLayer* get)
{
    bool changed = false;

    if (ImGui::TreeNode(UniqueUIName("Flat Lens"))) {
        // float sliders
        auto film = dynamic_cast<FlatLens*>(get);

        // Slider control the center position
        changed |= ImGui::SliderFloat2(
            UniqueUIName("Center"), film->center_pos.data(), -40, 40);
        changed |= ImGui::SliderFloat(
            UniqueUIName("Diameter"), &film->diameter, 0, 20);
        ImGui::TreePop();
    }
    return changed;
}

BBox2D SensorPainter::get_bounds(LensLayer* layer)
{
    // treat like flat lens
    auto sensor = dynamic_cast<Sensor*>(layer);

    auto diameter = sensor->diameter;

    return BBox2D{
        pxr::GfVec2f(
            sensor->center_pos[0], sensor->center_pos[1] - diameter / 2),
        pxr::GfVec2f(
            sensor->center_pos[0] + 1, sensor->center_pos[1] + diameter / 2),
    };
}

void SensorPainter::draw(
    DiffOpticsGUI* gui,
    LensLayer* layer,
    const pxr::GfMatrix3f& transform)
{
    auto sensor = dynamic_cast<Sensor*>(layer);
    auto center_pos = sensor->center_pos;
    auto diameter = sensor->diameter;
    auto ep1 = pxr::GfVec2f(center_pos[0], center_pos[1] + diameter / 2);
    auto ep2 = pxr::GfVec2f(center_pos[0], center_pos[1] - diameter / 2);
    auto tep1 = transform * pxr::GfVec3f(ep1[0], ep1[1], 1);
    auto tep2 = transform * pxr::GfVec3f(ep2[0], ep2[1], 1);
    gui->DrawLine(ImVec2(tep1[0], tep1[1]), ImVec2(tep2[0], tep2[1]));
}

bool SensorPainter::control(DiffOpticsGUI* diff_optics_gui, LensLayer* get)
{  // float sliders
    bool changed = false;

    if (ImGui::TreeNode(UniqueUIName("Sensor"))) {
        auto sensor = dynamic_cast<Sensor*>(get);
        // Slider control the center position
        changed |= ImGui::SliderFloat2(
            UniqueUIName("Center"), sensor->center_pos.data(), -40, 40);
        // Slider control the diameter
        changed |= ImGui::SliderFloat(
            UniqueUIName("Diameter"), &sensor->diameter, 0, 50);
        ImGui::TreePop();
    }

    return changed;
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

    //  Draw rays
    draw_rays(gui, transform);
}

void LensSystemGUI::draw_rays(DiffOpticsGUI* gui, const pxr::GfMatrix3f& t)
    const
{
    std::vector<RayInfo> begin_rays;

    const int ray_per_group = 5;
    const float group_spacing = 3.0f / ray_per_group;

    auto add_rays = [&](float origin_offset, float direction_x) {
        for (int i = 0; i < ray_per_group; ++i) {
            RayInfo ray;
            ray.Origin = { origin_offset + i * group_spacing, 0, -2 };
            ray.Direction = pxr::GfVec3f{ direction_x, 0, 1 }.GetNormalized();
            ray.TMin = 0;
            ray.TMax = 1000;
            ray.throughput.data = pxr::GfVec3f{ 1, 1, 1 };
            ray.throughput.padding = 0;
            begin_rays.push_back(ray);
        }
    };

    add_rays(-2.0f, 0);
    add_rays(-2.0f, 0.2f);
    add_rays(-7.0f, 0.4f);

    auto rays = lens_system->trace_ray(begin_rays);

    for (auto& ray_step : rays) {
        for (auto& ray : ray_step) {
            auto beg = ray.Origin;
            auto end = ray.Origin + ray.Direction * ray.TMax;
            auto tbeg = t * pxr::GfVec3f(beg[2], beg[0], 1);
            auto tend = t * pxr::GfVec3f(end[2], end[0], 1);
            gui->DrawLine(ImVec2(tbeg[0], tbeg[1]), ImVec2(tend[0], tend[1]));
        }
    }
}

void LensSystemGUI::control(DiffOpticsGUI* diff_optics_gui)
{
    // For each lens, give a imgui subgroup to control the lens

    bool changed = false;
    for (auto&& lens_layer : lens_system->lenses) {
        changed |=
            lens_layer->painter->control(diff_optics_gui, lens_layer.get());
    }
}
USTC_CG_NAMESPACE_CLOSE_SCOPE