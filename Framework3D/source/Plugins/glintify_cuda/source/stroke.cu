
#include <glm/glm.hpp>

#include "glintify/stroke.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace stroke {

// Another question would be how to consider the luminance? the shading?
// By controlling the density of the scratches.
// But how does that mean exactly?
HOST_DEVICE glm::vec2 Stroke::eval_required_direction(
    glm::vec2 uv_space_pos,
    glm::vec3 light_pos)
{
    auto uv_space_vpt_pos = world_to_tangent_point(virtual_point_position);

    glm::vec3 tangent_space_cam_dir =
        uv_space_vpt_pos - glm::vec3(uv_space_pos, 0);
    if (uv_space_vpt_pos.z > 0) {
        tangent_space_cam_dir *= -1;
    }

    glm::vec3 tangent_space_light_dir =
        world_to_tangent_point(light_pos) - glm::vec3(uv_space_pos, 0);

    glm::vec<3, float> half_vec = glm::normalize(
        0.5f * (glm::normalize(tangent_space_cam_dir) +
                glm::normalize(tangent_space_light_dir)));

    return glm::vec2(-half_vec.y, half_vec.x);
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

    auto tangent_space_light_pos = world_to_tangent_point(light_pos);

    float half_stroke_width = stroke_width / 2.0f;

    unsigned valid_sample_count = 0;

    glm::vec2 center_point;

    center_point.y = range[0].first.y;

    auto uv_vpt = world_to_tangent_point(virtual_point_position);

    uv_vpt.y = 2.0f * center_point.y - uv_vpt.y;

    glm::vec2 that_direction = uv_vpt - tangent_space_light_pos;
    center_point.x = tangent_space_light_pos.x +
                     (center_point.y - tangent_space_light_pos.y) *
                         that_direction.x / that_direction.y;

    auto vertical_movement =
        glm::vec2(0, 1) * stroke_width *
        (float(scratch_index / 2 + 0.5f) / float(MAX_SCRATCH_COUNT) - 0.25f) *
        90.0f;

    auto pos = center_point + glm::vec2(0.0001, 0) +
               glm::vec2(-1, 0) * float(scratch_index + 0.1f) /
                   float(MAX_SCRATCH_COUNT);

    pos = center_point + vertical_movement;

    glm::vec2 old_dir;

    for (int i = 0; i < SAMPLE_POINT_COUNT; ++i) {
        scratches[scratch_index].should_begin_new_line_mask[i] = false;
    }

    for (int i = 0; i < TEST_STEP_COUNT; ++i) {
        auto dir = eval_required_direction(pos, light_pos);

        if (i == 0) {
            auto scratch_going_right = dir.x > 0;
            if (!scratch_going_right) {
                dir *= -1;
            }
            bool other_way = scratch_index % 2 == 1;
            if (other_way) {
                dir *= -1;
            }
        }
        else {
            dir = same_direction(dir, old_dir);
        }

        old_dir = dir;

        if (std::abs(dir.y) > 0.999) {
            break;
        }

        auto step = 2.0f / float(TEST_STEP_COUNT);
        scratches[scratch_index].sample_point[valid_sample_count] = pos;

        auto temp_pos = pos;

        constexpr int substep_count = 10;

        auto sub_step = step / substep_count;

        for (int substep = 0; substep < substep_count; ++substep) {
            temp_pos += dir * sub_step;

            dir = eval_required_direction(temp_pos, light_pos);
            dir = same_direction(dir, old_dir);
        }

        pos = temp_pos;

        bool not_in_any_range = true;

        for (int j = 0; j < range_count; ++j) {
            auto left_point = range[j].first;
            auto right_point = range[j].second;

            if (left_point.x > right_point.x) {
                auto temp = left_point;
                left_point = right_point;
                right_point = temp;
            }

            if (pos.x >= left_point.x && pos.x <= right_point.x) {
                not_in_any_range = false;
                break;
            }
        }

        if (not_in_any_range) {
            scratches[scratch_index]
                .should_begin_new_line_mask[valid_sample_count] = true;

            continue;
        }

        if (pos.y < center_point.y - half_stroke_width ||
            pos.y > center_point.y + half_stroke_width) {
            scratches[scratch_index]
                .should_begin_new_line_mask[valid_sample_count] = true;

            continue;
        }
        valid_sample_count++;
        if (valid_sample_count >= SAMPLE_POINT_COUNT) {
            printf("Early stop because of too many samples\n");
            break;
        }
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
                    (0 - tangent_camera_left.z) /
                    (tangent_vpt.z - tangent_camera_left.z) +
                tangent_camera_left;

            glm::vec2 on_image_right =
                (tangent_vpt - tangent_camera_right) *
                    (0 - tangent_camera_right.z) /
                    (tangent_vpt.z - tangent_camera_right.z) +
                tangent_camera_right;

            if (on_image_left.x > on_image_right.x) {
                auto temp = on_image_left;
                on_image_left = on_image_right;
                on_image_right = temp;
            }

            stroke->range_count = 1;
            stroke->range[0] = std::make_pair(on_image_left, on_image_right);
        });
}
}  // namespace stroke

USTC_CG_NAMESPACE_CLOSE_SCOPE