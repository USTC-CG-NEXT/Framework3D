#pragma once
#include <pxr/usd/usd/stage.h>

struct GeomPayload {
    pxr::UsdStageRefPtr stage;
    pxr::SdfPath prim_path;

    float delta_time = 0.0f;
    bool has_simulation = false;
    bool is_simulating = false;
    pxr::UsdTimeCode current_time = pxr::UsdTimeCode::Default();
};