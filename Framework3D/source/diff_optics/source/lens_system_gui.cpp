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

void NullPainter::control(DiffOpticsGUI* diff_optics_gui, LensLayer* get)
{
    // Slider control the center position

    ImGui::SliderFloat2(
        UniqueUIName("Center"), get->center_pos.data(), -40, 40);
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
USTC_CG_NAMESPACE_CLOSE_SCOPE