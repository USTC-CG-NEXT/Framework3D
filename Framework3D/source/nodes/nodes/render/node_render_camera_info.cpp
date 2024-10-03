#include "NODES_FILES_DIR.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "Nodes/socket_types/buffer_socket_types.hpp"
#include "RCore/Backend.hpp"
#include "nvrhi/utils.h"
#include "render_node_base.h"
#include "resource_allocator_instance.hpp"

namespace USTC_CG::node_render_camera_info {
static void node_declare(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Camera>("Camera");
    b.add_output<decl::Float1Buffer>("Rotation Matrix");
    b.add_output<decl::Float1Buffer>("Translation");
    b.add_output<decl::Float>("FOV x");
    b.add_output<decl::Float>("FOV y");
    b.add_output<decl::Int>("X resolution");
    b.add_output<decl::Int>("Y resolution");

    b.add_output<decl::Float1Buffer>("world_view_transform");
    b.add_output<decl::Float1Buffer>("projection_matrix");
    b.add_output<decl::Float1Buffer>("full_proj_transform");
    b.add_output<decl::Float1Buffer>("camera_center");
}

static void node_exec(ExeParams params)
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

        GfMatrix4d projection = camera->ComputeProjectionMatrix();
        // Inverse compute the FOV from the projection matrix, both vertical and
        // horizontal

        // Compute FOV x and FOV y from the projection matrix
        double fov_x = 2 * atan(1.0 / projection[0][0]);
        double fov_y = 2 * atan(1.0 / projection[1][1]);

        // Set FOV x
        params.set_output("FOV x", static_cast<float>(fov_x));

        // Set FOV y
        params.set_output("FOV y", static_cast<float>(fov_y));

        auto transform = camera->GetTransform();
        auto world_view_transform = transform.GetInverse();
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
}

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "render_camera_info");
    strcpy(ntype.id_name, "node_render_camera_info");

    render_node_type_base(&ntype);
    ntype.node_execute = node_exec;
    ntype.declare = node_declare;
    nodeRegisterType(&ntype);
}

NOD_REGISTER_NODE(node_register)
}  // namespace USTC_CG::node_render_camera_info
