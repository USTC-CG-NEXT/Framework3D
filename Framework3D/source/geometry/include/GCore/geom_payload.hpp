#pragma once

#include <pxr/usd/usd/stage.h>

struct GeomPayload {
    pxr::UsdStageRefPtr stage;
    pxr::SdfPath prim_path;
};