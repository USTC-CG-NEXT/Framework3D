#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#ifndef __CUDACC_RTC__
#include <RHI/internal/cuda_extension.hpp>
#include <utility>
using std::pair;

#else
#define USTC_CG_NAMESPACE_OPEN_SCOPE  namespace USTC_CG {
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }
#define HOST_DEVICE                   __host__ __device__
#include <cuda/std/utility>
using cuda::std::pair;
#endif

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace stroke {

#define SAMPLE_POINT_COUNT 512

class Scratch {
   public:
    glm::vec2 sample_point[SAMPLE_POINT_COUNT];

    bool should_begin_new_line_mask[SAMPLE_POINT_COUNT];

    unsigned int valid_sample_count = 0;
};

#define MAX_SCRATCH_COUNT 128

#define MAX_RANGES 10

class Stroke {
   public:
    glm::vec3 virtual_point_position;

    float stroke_width = 0.005f;

    unsigned int scratch_count = 0;

    Scratch scratches[MAX_SCRATCH_COUNT];

    pair<glm::vec2, glm::vec2> range[MAX_RANGES];
    unsigned int range_count = 0;

    HOST_DEVICE void calc_seeds();
    /**
     * A function to calculate the tangent position of a point
     * @param world World position of the point
     * @return The tangent space position of the point
     */
    HOST_DEVICE glm::vec3 world_to_tangent_point(
        glm::vec3 world)  // A default implementation
    {
        return world * 0.5f + glm::vec3(0.5f, 0.5f, 0.0f);
    }

    HOST_DEVICE glm::vec3 world_to_tangent_vector(glm::vec3 world)
    {
        return world;
    }

    /**
     * A function to calculate the world position of a point
     * @param tangent Tangent space position of the point
     * @return The world position of the point
     */
    HOST_DEVICE glm::vec3 tangent_to_world_point(glm::vec3 tangent)
    {
        return (tangent - glm::vec3(0.5f, 0.5f, 0.0f)) * 2.0f;
    }

    HOST_DEVICE glm::vec3 tangent_to_world_vector(glm::vec3 tangent)
    {
        return tangent;
    }
    HOST_DEVICE glm::vec2 eval_required_direction(
        glm::vec2 uv_space_pos,
        glm::vec3 light_pos);

    HOST_DEVICE void calc_scratch(int scratch_index, glm::vec3 light_pos);

    void set_virtual_point_position(const glm::vec3 vec)
    {
        virtual_point_position = vec;
    }
};  // Relation to scratch: one to many
#ifndef __CUDACC_RTC__

void calc_scratches(
    cuda::CUDALinearBufferHandle strokes,
    glm::vec3 camera_position,
    glm::vec3 light_position);

// This one would be based on optix, raytracing to consider occlusions.

void calc_simple_plane_projected_ranges(
    const cuda::CUDALinearBufferHandle& d_strokes,
    glm::vec3 world_camera_position,
    glm::vec2 camera_move_range);

#endif
}  // namespace stroke

USTC_CG_NAMESPACE_CLOSE_SCOPE