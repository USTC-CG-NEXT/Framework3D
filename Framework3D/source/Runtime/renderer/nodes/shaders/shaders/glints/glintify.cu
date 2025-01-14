#include <optix_device.h>

#include "glintify/stroke.h"


#include "../Optix/ShaderNameAbbre.h"
#include "glintify/glintify_params.h"

inline unsigned GetLaunchID()
{
    uint3 launch_index = optixGetLaunchIndex();
    return launch_index.x;
}

__device__ float2 operator+(const float2& a, const float2& b)
{
    return make_float2(a.x + b.x, a.y + b.y);
}

__device__ float2 operator/(const float2& a, const float b)
{
    return make_float2(a.x / b, a.y / b);
}

float3 glm_to_float3(const glm::vec3& v)
{
    return make_float3(v.x, v.y, v.z);
}

RGS(mesh_glintify)
{
    auto id = GetLaunchID();
    auto stroke = params.strokes[id];

    auto camera_move_range = params.camera_move_range;

    constexpr unsigned sample_count = 256;

    auto camera_left = params.camera_position;
    camera_left.x += camera_move_range.x;

    auto camera_right = params.camera_position;
    camera_right.x += camera_move_range.y;

    for (int i = 0; i < sample_count; i++) {
        auto t = static_cast<float>(i) / (sample_count - 1);
        auto test_cam_pos = camera_left * (1 - t) + camera_right * t;

        auto dir = stroke.virtual_point_position - test_cam_pos;

        unsigned occluded = 0;
        optixTrace(
            params.handle,
            glm_to_float3(test_cam_pos),
            glm_to_float3(dir),
            0.0f,
            1.f,
            0.0f,
            OptixVisibilityMask(255),
            OPTIX_RAY_FLAG_NONE,
            0,
            1,
            0,
            occluded);
    }

    //auto tangent_vpt =
    //    stroke->world_to_tangent_point(stroke->virtual_point_position);

    //auto tangent_camera_left = stroke->world_to_tangent_point(camera_left);

    //auto tangent_camera_right = stroke->world_to_tangent_point(camera_right);

    //glm::vec2 on_image_left = (tangent_vpt - tangent_camera_left) *
    //                              (0 - tangent_camera_left.z) /
    //                              (tangent_vpt.z - tangent_camera_left.z) +
    //                          tangent_camera_left;

    //glm::vec2 on_image_right = (tangent_vpt - tangent_camera_right) *
    //                               (0 - tangent_camera_right.z) /
    //                               (tangent_vpt.z - tangent_camera_right.z) +
    //                           tangent_camera_right;

    //for (int i = 0; i < sample_count; i++) {
    //    auto t = static_cast<float>(i) / (sample_count - 1);
    //    auto on_image = on_image_left * (1 - t) + on_image_right * t;
    //}

    //stroke->range_count = 1;
    //stroke->range[0] = std::make_pair(on_image_left, on_image_right);
}

CHS(mesh_glintify)
{
    unsigned hit = 1;
    optixSetPayload_0(hit);
    printf("hit\n");

}
MISS(mesh_glintify)
{
}
