#include <glintify/glintify.hpp>

#include "stroke.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace stroke {

// Another question would be how to consider the luminance? the shading?
// By controlling the density of the scratches.
// But how does that mean exactly?

HOST_DEVICE glm::vec3 Stroke::world_to_tangent_point(
    glm::vec3 world)  // A default implementation
{
    return world * 0.5f + glm::vec3(0.5f, 0.5f, 0.0f);
}

HOST_DEVICE glm::vec3 Stroke::world_to_tangent_vector(glm::vec3 world)
{
    return world;
}

HOST_DEVICE glm::vec3 Stroke::tangent_to_world_point(glm::vec3 tangent)
{
    return (tangent - glm::vec3(0.5f, 0.5f, 0.0f)) * 2.0f;
}

HOST_DEVICE glm::vec3 Stroke::tangent_to_world_vector(glm::vec3 tangent)
{
    return tangent;
}

HOST_DEVICE glm::vec2 Stroke::eval_required_direction(
    glm::vec2 uv_space_pos,
    glm::vec3 light_pos)
{
    auto uv_space_vpt_pos = world_to_tangent_point(virtual_point_position);

    glm::vec2 tangent_space_cam_dir =
        uv_space_vpt_pos - glm::vec3(uv_space_pos, 0);
    if (uv_space_vpt_pos.z < 0) {
        tangent_space_cam_dir *= -1;
    }

    glm::vec2 tangent_space_light_dir =
        world_to_tangent_point(light_pos) - glm::vec3(uv_space_pos, 0);

    auto half_vec = 0.5f * (glm::normalize(tangent_space_cam_dir) +
                            glm::normalize(tangent_space_light_dir));

    return glm::normalize(glm::vec2(-half_vec.y, half_vec.x));
}

HOST_DEVICE glm::vec2 same_direction(glm::vec2 vec, glm::vec2 reference)
{
    if (glm::dot(vec, reference) < 0) {
        return -vec;
    }
    return vec;
}

HOST_DEVICE void Stroke::calc_scratch(int scratch_index, glm::vec3 light_pos)
{
    scratch_count = MAX_SCRATCH_COUNT;

    auto left_point = range[0].first;
    auto right_point = range[0].second;

    if (left_point.x > right_point.x) {
        auto temp = left_point;
        left_point = right_point;
        right_point = temp;
    }

    auto tangent_space_light_pos = world_to_tangent_point(light_pos);

    float half_stroke_width = stroke_width / 2.0f;

    unsigned valid_sample_count = 0;

    glm::vec2 center_point;

    center_point.y = left_point.y;

    auto uv_vpt = world_to_tangent_point(virtual_point_position);

    uv_vpt.y = 2.0f * center_point.y - uv_vpt.y;

    glm::vec2 that_direction = uv_vpt - tangent_space_light_pos;
    center_point.x = tangent_space_light_pos.x +
                     (center_point.y - tangent_space_light_pos.y) *
                         that_direction.x / that_direction.y;

    auto pos = center_point + glm::vec2(-1, 0) * float(scratch_index + 0.5f) /
                                  float(MAX_SCRATCH_COUNT) / 1.f;

    glm::vec2 old_dir;

    for (int i = 0; i < SAMPLE_POINT_COUNT; ++i) {
        scratches[scratch_index].should_begin_new_line_mask[i] = false;
    }

    for (int i = 0; i < SAMPLE_POINT_COUNT; ++i) {
        auto dir = eval_required_direction(pos, light_pos);

        if (i == 0) {
            auto scratch_going_right = dir.x > 0;
            if (!scratch_going_right) {
                dir *= -1;
            }

            bool scratch_going_upward = dir.y > 0;
            if (scratch_going_upward) {
                pos.y -= half_stroke_width;
            }
            else {
                pos.y += half_stroke_width;
            }
        }
        else {
            dir = same_direction(dir, old_dir);
        }

        old_dir = dir;

        if (std::abs(dir.y) > 0.999) {
            break;
        }

        auto step = stroke_width / float(SAMPLE_POINT_COUNT) * 50.f;

        pos += dir * step;

        if (pos.x < left_point.x || pos.x > right_point.x) {
            scratches[scratch_index]
                .should_begin_new_line_mask[valid_sample_count] = true;

            continue;
        }

        if (pos.y < left_point.y - half_stroke_width ||
            pos.y > right_point.y + half_stroke_width) {
            scratches[scratch_index]
                .should_begin_new_line_mask[valid_sample_count] = true;
            continue;
        }

        scratches[scratch_index].sample_point[valid_sample_count] = pos;
        valid_sample_count++;
    }

    scratches[scratch_index].valid_sample_count = valid_sample_count;

    // if (scratch_index == 0) {
    //     scratches[0].sample_point[0] = center_point;
    //     scratches[0].sample_point[1] = center_point + glm::vec2(0, -1);
    // }
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
                    (tangent_camera_left.z) /
                    (tangent_vpt.z - tangent_camera_left.z) +
                tangent_camera_left;

            glm::vec2 on_image_right =
                (tangent_vpt - tangent_camera_right) *
                    (tangent_camera_right.z) /
                    (tangent_vpt.z - tangent_camera_right.z) +
                tangent_camera_right;

            stroke->range_count = 1;
            stroke->range[0] = std::make_pair(on_image_left, on_image_right);
        });
}
}  // namespace stroke

USTC_CG_NAMESPACE_CLOSE_SCOPE