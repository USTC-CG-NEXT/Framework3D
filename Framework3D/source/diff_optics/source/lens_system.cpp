#define IMGUI_DEFINE_MATH_OPERATORS

#include "diff_optics/lens_system.hpp"

#include <diff_optics/api.h>

#include <memory>
#include <vector>

#include "dO_GUI.hpp"
#include "imgui.h"
#include "pxr/base/gf/vec2f.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
BBox2D::BBox2D() : min(1e9, 1e9), max(-1e9, -1e9)
{
}

BBox2D::BBox2D(ImVec2 min, ImVec2 max) : min(min), max(max)
{
}

ImVec2 BBox2D::center() const
{
    return (min + max) / 2;
}

BBox2D BBox2D::operator+(const BBox2D& b) const
{
    // Bounding box merge
    return BBox2D{
        ImVec2(std::min(min.x, b.min.x), std::min(min.y, b.min.y)),
        ImVec2(std::max(max.x, b.max.x), std::max(max.y, b.max.y)),
    };
}

BBox2D& BBox2D::operator+=(const BBox2D& b)
{
    min.x = std::min(min.x, b.min.x);
    min.y = std::min(min.y, b.min.y);
    max.x = std::max(max.x, b.max.x);
    max.y = std::max(max.y, b.max.y);
    return *this;
}

LensLayer::LensLayer(float center_x, float center_y)
    : center_pos(center_x, center_y)
{
}

LensLayer::~LensLayer() = default;

void LensLayer::EmitShader()
{
}

void LensLayer::set_axis(float axis_pos)
{
    center_pos.y = axis_pos;
}

void LensLayer::set_pos(float x)
{
    center_pos.x = x;
}

Pupil::Pupil(float radius, float x, float y) : radius(radius), LensLayer(x, y)
{
    painter = std::make_shared<PupilPainter>();
}

BBox2D PupilPainter::get_bounds(LensLayer* layer)
{
    auto pupil = dynamic_cast<Pupil*>(layer);
    auto radius = pupil->radius;
    auto center_pos = pupil->center_pos;
    return BBox2D{
        ImVec2(center_pos.x, center_pos.y - radius - 1),
        ImVec2(center_pos.x + 1, center_pos.y + radius + 1),
    };
}

void PupilPainter::draw(
    DiffOpticsGUI* gui,
    LensLayer* layer,
    const pxr::GfMatrix3f& xform)
{
    auto pupil = dynamic_cast<Pupil*>(layer);

    pxr::GfVec2f ep1 = { 0, pupil->radius };
    pxr::GfVec2f ep2 = { 0, pupil->radius + 1 };
    pxr::GfVec2f ep3 = { 0, -pupil->radius };
    pxr::GfVec2f ep4 = { 0, -pupil->radius - 1 };

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

BBox2D LensFilmPainter::get_bounds(LensLayer* layer)
{
}

LensSystem::LensSystem() : gui(std::make_unique<LensSystemGUI>(this))
{
}

void LensSystemGUI::set_canvas_size(float x, float y)
{
    canvas_size = ImVec2(x, y);
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
    auto canvas_center = ImVec2(canvas_size.x / 2, canvas_size.y / 2);

    auto scale = std::min(
        canvas_size.x / (bound.max.x - bound.min.x),
        canvas_size.y / (bound.max.y - bound.min.y));

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
            canvas_center.x - bound_center.x,
            canvas_center.y - bound_center.y,
            1,
        }
            .GetTranspose();

    for (auto& lens : lens_system->lenses) {
        lens->painter->draw(gui, lens.get(), transform);
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
