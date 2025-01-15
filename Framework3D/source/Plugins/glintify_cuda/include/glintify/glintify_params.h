#pragma once
#include "glintify/stroke.h"

struct GlintifyParams {
    OptixTraversableHandle handle;
    USTC_CG::stroke::Stroke** strokes;
    glm::vec3 camera_position;
    glm::vec2 camera_move_range;
};

extern "C" {
extern __constant__ GlintifyParams params;
}
