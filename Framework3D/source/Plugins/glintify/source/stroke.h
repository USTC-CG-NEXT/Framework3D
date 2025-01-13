#pragma once

#include <glintify/api.h>

#include <RHI/internal/cuda_extension.hpp>
#include <glm/glm.hpp>
USTC_CG_NAMESPACE_OPEN_SCOPE

#define SAMPLE_POINTS 64

class Scratch {
    glm::vec2 begin[SAMPLE_POINTS];
    glm::vec2 end[SAMPLE_POINTS];
};

#define MAX_SCRATCH_COUNT 128

#define MAX_RANGES 10

class Stroke {
    glm::vec3 cam_local_pos;
    glm::vec3 light_local_pos;

    unsigned int scratch_count;

    Scratch scratches[MAX_SCRATCH_COUNT];
    float init_seed[MAX_SCRATCH_COUNT];
    glm::vec2 range[MAX_RANGES];

    HOST_DEVICE void calc_seeds();

};  // Relation to scratch: one to many

USTC_CG_NAMESPACE_CLOSE_SCOPE