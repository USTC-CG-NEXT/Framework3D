#pragma once

#include <pxr/usd/usd/stage.h>

#include <string>

#include "polyscope_widget/polyscope_renderer.h"


struct PolyscopeWidgetPayload {
    pxr::UsdStageRefPtr stage;
    pxr::SdfPath prim_path;
    std::string renderer_window_name;
};