
#include "nodes/core/def/node_def.hpp"
#include "nvrhi/nvrhi.h"
#include "pxr/base/gf/matrix3d.h"
#include "render_node_base.h"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(camera_info)
{
    b.add_output<pxr::VtArray<float>>("Rotation Matrix");
    b.add_output<pxr::VtArray<float>>("Translation");
    b.add_output<float>("FOV x");
    b.add_output<float>("FOV y");
    b.add_output<int>("X resolution");
    b.add_output<int>("Y resolution");

    b.add_output<pxr::VtArray<float>>("world_view_transform");
    b.add_output<pxr::VtArray<float>>("projection_matrix");
    b.add_output<pxr::VtArray<float>>("full_proj_transform");
    b.add_output<pxr::VtArray<float>>("camera_center");
}

NODE_EXECUTION_FUNCTION(camera_info)
{
    Hd_USTC_CG_Camera* camera = get_free_camera(params);

    using namespace pxr;

    if (camera) {
        GfMatrix3d rotation = camera->GetTransform().ExtractRotationMatrix();
        VtArray<float> rotation_array(9);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                rotation_array[i * 3 + j] = rotation[i][j];
            }
        }
        // Set Rotation Matrix
        params.set_output("Rotation Matrix", rotation_array);

        auto translation = camera->GetTransform().ExtractTranslation();
        VtArray<float> translation_vec(3);
        for (int i = 0; i < 3; i++) {
            translation_vec[i] = translation[i];
        }
        // Set Translation Matrix
        params.set_output("Translation", translation_vec);

        GfMatrix4f projection = camera->projMatrix;
        // Inverse compute the FOV from the projection matrix, both vertical and
        // horizontal

        // Compute FOV x and FOV y from the projection matrix
        double fov_x = 2 * atan(1.0 / projection[0][0]);
        double fov_y = 2 * atan(1.0 / projection[1][1]);

        // Set FOV x
        params.set_output("FOV x", static_cast<float>(fov_x));

        // Set FOV y
        params.set_output("FOV y", static_cast<float>(fov_y));

        auto world_view_transform = camera->viewMatrix;
        VtArray<float> world_view_transform_array(16);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                world_view_transform_array[i * 4 + j] =
                    world_view_transform[i][j];
            }
        }
        // Set world_view_transform
        params.set_output("world_view_transform", world_view_transform_array);

        VtArray<float> projection_matrix(16);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                projection_matrix[i * 4 + j] = projection[i][j];
            }
        }

        // Set projection_matrix
        params.set_output("projection_matrix", projection_matrix);

        auto full_projection_transform = world_view_transform * projection;
        VtArray<float> full_proj_transform(16);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                full_proj_transform[i * 4 + j] =
                    full_projection_transform[i][j];
            }
        }

        // Set full_proj_transform
        params.set_output("full_proj_transform", full_proj_transform);

        // Set camera_center
        params.set_output("camera_center", translation_vec);

        auto x_resolution = camera->dataWindow.GetSize()[0];
        auto y_resolution = camera->dataWindow.GetSize()[1];

        params.set_output("X resolution", x_resolution);
        params.set_output("Y resolution", y_resolution);
    }
    return true;
}

NODE_DECLARATION_UI(camera_info);
NODE_DEF_CLOSE_SCOPE
