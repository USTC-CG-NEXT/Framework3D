#include <optix_device.h>

#include "../Optix/ShaderNameAbbre.h"
#include "glintify/glintify_params.h"
#include "glintify/stroke.h"

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

    constexpr unsigned sample_count = 1024;

    auto camera_left = params.camera_position;
    camera_left.x += camera_move_range.x;


    auto camera_right = params.camera_position;
    camera_right.x += camera_move_range.y;

    auto tangent_vpt =
        stroke->world_to_tangent_point(stroke->virtual_point_position);

    unsigned current_range = 0;

    bool taping = false;
    glm::vec2 on_image;

    for (int i = 0; i < sample_count; i++) {
        auto t = static_cast<float>(i) / (sample_count - 1);
        auto test_cam_pos =
            camera_left * (1 - t) + camera_right * t;


        auto dir = stroke->virtual_point_position - test_cam_pos;

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

        //occluded = 0;
         
        bool start_taping = !taping && !occluded;
        bool end_taping = taping && occluded;
        
        test_cam_pos = stroke->world_to_tangent_point(test_cam_pos);


        on_image = (tangent_vpt - test_cam_pos) * (0 - test_cam_pos.z) /
                       (tangent_vpt.z - test_cam_pos.z) +
                   test_cam_pos;

        if (start_taping) {
            taping = true;
            stroke->range[current_range].first = on_image;
        }

        if (end_taping) {
            taping = false;
            stroke->range[current_range].second = on_image;

            current_range++;
        }
    }

    if (taping) {
        stroke->range[current_range].second = on_image;
        current_range++;
    }

    stroke->range_count = current_range;

    // camera_left = params.camera_position;
    //         camera_left.x += camera_move_range.x;
    //         auto tangent_camera_left =
    //             stroke->world_to_tangent_point(camera_left);

    //        camera_right = params.camera_position;
    //        camera_right.x += camera_move_range.y;

    //        auto tangent_camera_right =
    //            stroke->world_to_tangent_point(camera_right);

    //        glm::vec2 on_image_left =
    //            (tangent_vpt - tangent_camera_left) *
    //                (0 - tangent_camera_left.z) /
    //                (tangent_vpt.z - tangent_camera_left.z) +
    //            tangent_camera_left;

    //        glm::vec2 on_image_right =
    //            (tangent_vpt - tangent_camera_right) *
    //                (0 - tangent_camera_right.z) /
    //                (tangent_vpt.z - tangent_camera_right.z) +
    //            tangent_camera_right;

    //        if (on_image_left.x > on_image_right.x) {
    //            auto temp = on_image_left;
    //            on_image_left = on_image_right;
    //            on_image_right = temp;
    //        }

    //        stroke->range_count = 1;
    //        stroke->range[0] = cuda::std::make_pair(on_image_left,
    //        on_image_right);
}

CHS(mesh_glintify)
{
    unsigned hit = 1;
    optixSetPayload_0(hit);
}
MISS(mesh_glintify)
{
}
