#pragma once

#include <glintify/api.h>

#include <RHI/internal/cuda_extension.hpp>
#include <glm/glm.hpp>
USTC_CG_NAMESPACE_OPEN_SCOPE

namespace stroke {

#define SAMPLE_POINT_COUNT 16

class Scratch {
   public:
    glm::vec2 sample_point[SAMPLE_POINT_COUNT];
};

#define MAX_SCRATCH_COUNT 128

#define MAX_RANGES 10

class Stroke {
   public:
    glm::vec3 cam_local_pos;
    glm::vec3 light_local_pos;
    glm::vec3 virtual_point_position;

    unsigned int scratch_count = 0;

    Scratch scratches[MAX_SCRATCH_COUNT];
    float init_seed[MAX_SCRATCH_COUNT];
    glm::vec2 range[MAX_RANGES];

    HOST_DEVICE void calc_seeds();

    __device__ void calc_scratch(int scratch_index);

    void set_virtual_point_position(const glm::vec3 vec)
    {
        virtual_point_position = vec;
    }
};  // Relation to scratch: one to many

void calc_scratches(
    cuda::CUDALinearBufferHandle strokes,
    glm::vec3 camera_position,
    glm::vec3 light_position);

}  // namespace stroke

USTC_CG_NAMESPACE_CLOSE_SCOPE