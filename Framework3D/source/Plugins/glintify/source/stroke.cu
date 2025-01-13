#include <glintify/glintify.hpp>

#include "stroke.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace stroke {

// Another question would be how to consider the luminance? the shading?
// By controlling the density of the scratches.
// But how does that mean exactly?

__device__ glm::vec3 Stroke::world_to_tangent_point(
    glm::vec3 world)  // A default implementation
{
    return world * 0.5f + glm::vec3(0.5f, 0.5f, 0.0f);
}

__device__ glm::vec3 Stroke::world_to_tangent_vector(glm::vec3 world)
{
    return world;
}

__device__ glm::vec3 Stroke::tangent_to_world_point(glm::vec3 tangent)
{
    return (tangent - glm::vec3(0.5f, 0.5f, 0.0f)) * 2.0f;
}

__device__ glm::vec3 Stroke::tangent_to_world_vector(glm::vec3 tangent)
{
    return tangent;
}

__device__ glm::vec2 Stroke::eval_required_direction(
    glm::vec2 uv_space_pos,
    glm::vec3 light_pos)
{
    auto uv_space_vpt_pos = world_to_tangent_point(virtual_point_position);

    auto tangent_space_cam_dir = uv_space_vpt_pos - glm::vec3(uv_space_pos, 0);

    auto tangent_space_light_dir =
        world_to_tangent_point(light_pos) - glm::vec3(uv_space_pos, 0);

    auto half_vec = 0.5f * (glm::normalize(tangent_space_cam_dir) +
                            glm::normalize(tangent_space_light_dir));

    return -glm::normalize(glm::vec2(-half_vec.y, half_vec.x));
}

__device__ glm::vec2 same_direction(glm::vec2 vec, glm::vec2 reference)
{
    if (glm::dot(vec, reference) < 0) {
        return -vec;
    }
    return vec;
}

__device__ void Stroke::calc_scratch(int scratch_index, glm::vec3 light_pos)
{
    scratch_count = MAX_SCRATCH_COUNT;

    auto left_point = range[0].first;
    auto right_point = range[0].second;

    if (left_point.x > right_point.x) {
        auto temp = left_point;
        left_point = right_point;
        right_point = temp;
    }

    float half_stroke_width = stroke_width / 2.0f;

    unsigned valid_sample_count = 0;

    auto beginner_dir = eval_required_direction(left_point, light_pos);
    auto center_point = left_point + (right_point - left_point) / 2.0f;

    auto ratio = std::abs(beginner_dir.y) / (std::abs(beginner_dir.x) + 0.01f);
    ratio = 1;

    auto case_id = scratch_index % 2;
    auto init_pos_step = scratch_index / 2;

    bool init_pos_going_right = case_id % 2 == 0;

    auto pos = center_point +
               (init_pos_going_right ? glm::vec2(1, 0) : glm::vec2(-1, 0)) *
                   ratio * float(init_pos_step) / float(MAX_SCRATCH_COUNT) /
                   2.0f * (right_point - left_point);

    glm::vec2 old_dir;

    bool allow_going_left = false;

    for (int i = 0; i < SAMPLE_POINT_COUNT; ++i) {
        auto dir = eval_required_direction(pos, light_pos);

        // if (!allow_going_left) {
        //     if (dir.x < 0) {
        //         break;
        //     }
        // }

        auto scratch_going_upward = dir.y > 0;

        if (scratch_going_upward) {
            pos.y -= half_stroke_width;
        }
        else {
            pos.y += half_stroke_width;
        }

        if (i == 0) {
            //if (scratch_going_upward) {
            //    dir *= dir.y > 0 ? 1 : -1;
            //}
            //else {
            //    dir *= dir.y > 0 ? -1 : 1;
            //}
        }
        else {
            dir = same_direction(dir, old_dir);
        }

        old_dir = dir;

        auto step =
            1 / float(SAMPLE_POINT_COUNT) / (std::abs(dir.x) + 0.1f) * 0.5f;

        step = 1;

        scratches[scratch_index].sample_point[i] = pos;
        valid_sample_count++;

        dir = glm::vec2(0, 1);

        pos += dir * step;

        // if (pos.x < left_point.x || pos.x > right_point.x) {
        //     break;
        // }

        // if (pos.y < left_point.y - half_stroke_width ||
        //     pos.y > right_point.y + half_stroke_width) {
        //     allow_going_left = true;
        //     break;
        // }
    }

    scratches[scratch_index].valid_sample_count = valid_sample_count;
}

void calc_scratches(
    cuda::CUDALinearBufferHandle strokes,
    glm::vec3 camera_position,
    glm::vec3 light_position)
{
    auto stroke_count = strokes->getDesc().element_count;

    unsigned calculation_load = stroke_count * MAX_SCRATCH_COUNT;

    Stroke** d_strokes_ptr =
        reinterpret_cast<Stroke**>(strokes->get_device_ptr());

    GPUParallelFor(
        "calc_scratches", calculation_load, GPU_LAMBDA_Ex(int index) {
            auto related_stroke = index / MAX_SCRATCH_COUNT;
            auto scratch_index = index % MAX_SCRATCH_COUNT;
            auto stroke = d_strokes_ptr[related_stroke];

            stroke->calc_scratch(scratch_index, light_position);
        });
}

void calc_simple_plane_projected_ranges(
    const cuda::CUDALinearBufferHandle& d_strokes,
    glm::vec3 world_camera_position,
    glm::vec2 camera_move_range)
{
    auto stroke_count = d_strokes->getDesc().element_count;
    Stroke** d_strokes_ptr =
        reinterpret_cast<Stroke**>(d_strokes->get_device_ptr());
    GPUParallelFor(
        "calc_simple_projected_ranges", stroke_count, GPU_LAMBDA_Ex(int index) {
            auto stroke = d_strokes_ptr[index];

            auto tangent_vpt =
                stroke->world_to_tangent_point(stroke->virtual_point_position);

            auto camera_left = world_camera_position;
            camera_left.x += camera_move_range.x;

            auto tangent_camera_left =
                stroke->world_to_tangent_point(camera_left);

            auto camera_right = world_camera_position;
            camera_right.x += camera_move_range.y;

            auto tangent_camera_right =
                stroke->world_to_tangent_point(camera_right);

            glm::vec2 on_image_left =
                (tangent_vpt - tangent_camera_left) *
                    abs(tangent_camera_left.z) /
                    (tangent_vpt.z - tangent_camera_left.z) +
                tangent_camera_left;

            glm::vec2 on_image_right =
                (tangent_vpt - tangent_camera_right) *
                    abs(tangent_camera_right.z) /
                    (tangent_vpt.z - tangent_camera_right.z) +
                tangent_camera_right;

            stroke->range_count = 1;
            stroke->range[0] = std::make_pair(on_image_left, on_image_right);
        });
}
}  // namespace stroke

USTC_CG_NAMESPACE_CLOSE_SCOPE