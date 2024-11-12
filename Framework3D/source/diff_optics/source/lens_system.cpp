#define IMGUI_DEFINE_MATH_OPERATORS

#include "diff_optics/lens_system.hpp"

#include <diff_optics/api.h>

#include <memory>
#include <vector>

#include "dO_GUI.hpp"
#include "imgui.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
Pupil::Pupil(float radius, float x, float y) : radius(radius), LensLayer(x, y)

{
    painter = std::make_shared<PupilPainter>();
}

void PupilPainter::draw(
    DiffOpticsGUI* gui,
    LensLayer* layer,
    ImVec2 overall_bias)
{
    auto pupil = dynamic_cast<Pupil*>(layer);
    auto radius = pupil->radius;
    gui->DrawLine(
        pupil->center_pos + overall_bias + ImVec2(0, radius),
        pupil->center_pos + overall_bias + ImVec2(0, radius + 5));

    gui->DrawLine(
        pupil->center_pos + overall_bias + ImVec2(0, -radius),
        pupil->center_pos + overall_bias + ImVec2(0, -radius - 5));
}

void LensSystem::draw(DiffOpticsGUI* gui) const
{
    ImVec2 overall_bias(0, 0);
    for (auto& lens : lenses) {
        lens->painter->draw(gui, lens.get(), overall_bias);
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
