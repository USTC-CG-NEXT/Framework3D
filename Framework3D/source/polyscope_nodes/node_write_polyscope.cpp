#include "GCore/Components/CurveComponent.h"
#include "GCore/Components/MaterialComponent.h"
#include "GCore/Components/MeshOperand.h"
#include "GCore/Components/PointsComponent.h"
#include "GCore/Components/XformComponent.h"
#include "GCore/GOP.h"
#include "GCore/geom_payload.hpp"
#include "glm/fwd.hpp"
#include "nodes/core/def/node_def.hpp"
#include "polyscope/curve_network.h"
#include "polyscope/image_quantity.h"
#include "polyscope/point_cloud.h"
#include "polyscope/structure.h"
#include "polyscope/surface_mesh.h"
#include "polyscope_widget/polyscope_renderer.h"
#include "pxr/base/gf/rotation.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(write_polyscope)
{
    b.add_input<Geometry>("Geometry");
    b.add_input<float>("Time Code").default_val(0).min(0).max(240);
}

bool legal(const std::string& string)
{
    if (string.empty()) {
        return false;
    }
    if (std::find_if(string.begin(), string.end(), [](char val) {
            return val == '(' || val == ')' || val == '-' || val == ',';
        }) == string.end()) {
        return true;
    }
    return false;
}

// TODO: Test and add support for materials and textures
// The current implementation has not been fully tested yet
NODE_EXECUTION_FUNCTION(write_polyscope)
{
    auto global_payload = params.get_global_payload<GeomPayload>();

    auto geometry = params.get_input<Geometry>("Geometry");

    auto mesh = geometry.get_component<MeshComponent>();

    auto points = geometry.get_component<PointsComponent>();

    auto curve = geometry.get_component<CurveComponent>();

    assert(!(points && mesh));

    auto t = params.get_input<float>("Time Code");
    pxr::UsdTimeCode time = pxr::UsdTimeCode(t);
    if (t == 0) {
        time = pxr::UsdTimeCode::Default();
    }

    auto stage = global_payload.stage;
    auto sdf_path = global_payload.prim_path;

    polyscope::Structure* structure = nullptr;

    if (mesh) {
        auto vertices = mesh->get_vertices();
        // faceVertexIndices是一个一维数组，每faceVertexCounts[i]个元素表示一个面
        auto faceVertexCounts = mesh->get_face_vertex_counts();
        auto faceVertexIndices = mesh->get_face_vertex_indices();
        auto display_color = mesh->get_display_color();
        // 转换为nested array
        std::vector<std::vector<size_t>> faceVertexIndicesNested;
        size_t start = 0;
        for (size_t i = 0; i < faceVertexCounts.size(); i++) {
            std::vector<size_t> face;
            for (size_t j = 0; j < faceVertexCounts[i]; j++) {
                face.push_back(faceVertexIndices[start + j]);
            }
            faceVertexIndicesNested.push_back(face);
            start += faceVertexCounts[i];
        }

        auto surface_mesh = polyscope::registerSurfaceMesh(
            sdf_path.GetName(), vertices, faceVertexIndicesNested);

        if (display_color.size() > 0) {
            try {
                surface_mesh->addVertexColorQuantity("color", display_color)
                    ->setEnabled(true);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                return false;
            }
        }

        if (mesh->get_vertex_scalar_quantities().size() > 0) {
            for (size_t i = 0; i < mesh->get_vertex_scalar_quantities().size();
                 i++) {
                try {
                    surface_mesh->addVertexScalarQuantity(
                        "vertex scalar" + std::to_string(i),
                        mesh->get_vertex_scalar_quantities()[i]);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
        }

        if (mesh->get_face_scalar_quantities().size() > 0) {
            for (size_t i = 0; i < mesh->get_face_scalar_quantities().size();
                 i++) {
                try {
                    surface_mesh->addFaceScalarQuantity(
                        "face scalar" + std::to_string(i),
                        mesh->get_face_scalar_quantities()[i]);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
        }

        if (mesh->get_vertex_color_quantities().size() > 0) {
            for (size_t i = 0; i < mesh->get_vertex_color_quantities().size();
                 i++) {
                try {
                    surface_mesh->addVertexColorQuantity(
                        "vertex color" + std::to_string(i),
                        mesh->get_vertex_color_quantities()[i]);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
        }

        if (mesh->get_face_color_quantities().size() > 0) {
            for (size_t i = 0; i < mesh->get_face_color_quantities().size();
                 i++) {
                try {
                    surface_mesh->addFaceColorQuantity(
                        "face color" + std::to_string(i),
                        mesh->get_face_color_quantities()[i]);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
        }

        if (mesh->get_vertex_vector_quantities().size() > 0) {
            for (size_t i = 0; i < mesh->get_vertex_vector_quantities().size();
                 i++) {
                try {
                    surface_mesh->addVertexVectorQuantity(
                        "vertex vector" + std::to_string(i),
                        mesh->get_vertex_vector_quantities()[i]);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
        }

        if (mesh->get_face_vector_quantities().size() > 0) {
            for (size_t i = 0; i < mesh->get_face_vector_quantities().size();
                 i++) {
                try {
                    surface_mesh->addFaceVectorQuantity(
                        "face vector" + std::to_string(i),
                        mesh->get_face_vector_quantities()[i]);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
        }

        if (mesh->get_face_corner_parameterization_quantities().size() > 0) {
            for (size_t i = 0;
                 i < mesh->get_face_corner_parameterization_quantities().size();
                 i++) {
                try {
                    surface_mesh->addParameterizationQuantity(
                        "face corner parameterization" + std::to_string(i),
                        mesh->get_face_corner_parameterization_quantities()[i]);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
        }

        if (mesh->get_vertex_parameterization_quantities().size() > 0) {
            for (size_t i = 0;
                 i < mesh->get_vertex_parameterization_quantities().size();
                 i++) {
                try {
                    surface_mesh->addVertexParameterizationQuantity(
                        "vertex parameterization" + std::to_string(i),
                        mesh->get_vertex_parameterization_quantities()[i]);
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
        }

        structure = surface_mesh;
    }
    else if (points) {
        auto vertices = points->get_vertices();
        auto display_color = points->get_display_color();
        auto width = points->get_width();

        auto point_cloud =
            polyscope::registerPointCloud(sdf_path.GetName(), vertices);

        if (width.size() > 0) {
            try {
                point_cloud->addScalarQuantity("width", width);
                point_cloud->setPointRadiusQuantity("width");
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                return false;
            }
        }

        if (display_color.size() > 0) {
            try {
                point_cloud->addColorQuantity("color", display_color)
                    ->setEnabled(true);
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                return false;
            }
        }

        structure = point_cloud;
    }
    else if (curve) {
        auto vertices = curve->get_vertices();
        // vert_count是一个一维数组，每个元素表示一个curve的点数，vertices中每vert_count[i]个元素表示一个curve
        auto vert_count = curve->get_vert_count();
        auto width = curve->get_width();
        auto display_color = curve->get_display_color();
        // 转换为edge array
        std::vector<std::array<size_t, 2>> edges;
        size_t start = 0;
        for (size_t i = 0; i < vert_count.size(); i++) {
            for (size_t j = 0; j < vert_count[i] - 1; j++) {
                edges.push_back({ start + j, start + j + 1 });
            }
            start += vert_count[i];
        }

        auto curve_network = polyscope::registerCurveNetwork(
            sdf_path.GetName(), vertices, edges);

        structure = curve_network;
    }

    if (!structure) {
        std::exception("No geometry found");
        return false;
    }

    // Material and Texture
    auto material_component = geometry.get_component<MaterialComponent>();
    // 目前只支持mesh
    if (material_component && mesh) {
        // 仅当有uv时才添加纹理
        if (mesh->get_vertex_parameterization_quantities().size() > 0 ||
            mesh->get_face_corner_parameterization_quantities().size() > 0) {
            // auto usdgeom = pxr::UsdGeomXformable ::Get(stage, sdf_path);
            if (legal(std::string(material_component->textures[0].c_str()))) {
                auto texture_name =
                    std::string(material_component->textures[0].c_str());
                int width, height, channels;
                unsigned char* data = stbi_load(
                    texture_name.c_str(), &width, &height, &channels, 4);
                if (!data) {
                    std::cerr << "failed to load image from " << texture_name
                              << std::endl;
                    return false;
                }
                bool has_alpha = (channels == 4);
                // Parse the data in to a float array
                std::vector<std::array<float, 3>> image_color(width * height);
                std::vector<std::array<float, 4>> image_color_alpha(
                    width * height);
                std::vector<float> image_scalar(width * height);
                for (int j = 0; j < height; j++) {
                    for (int i = 0; i < width; i++) {
                        int pix_ind = (j * width + i) * channels;
                        unsigned char p_r = data[pix_ind + 0];
                        unsigned char p_g = data[pix_ind + 1];
                        unsigned char p_b = data[pix_ind + 2];
                        unsigned char p_a = 255;
                        if (channels == 4) {
                            p_a = data[pix_ind + 3];
                        }

                        // color
                        std::array<float, 3> val{ p_r / 255.f,
                                                  p_g / 255.f,
                                                  p_b / 255.f };
                        image_color[j * width + i] = val;

                        // scalar
                        image_scalar[j * width + i] =
                            (val[0] + val[1] + val[2]) / 3.;

                        // color alpha
                        std::array<float, 4> val_a{
                            p_r / 255.f, p_g / 255.f, p_b / 255.f, p_a / 255.f
                        };
                        image_color_alpha[j * width + i] = val_a;
                    }
                }

                // 需要将structure转换为surface_mesh
                auto surface_mesh =
                    dynamic_cast<polyscope::SurfaceMesh*>(structure);

                try {
                    // surface_mesh->addColorImageQuantity(
                    //     texture_name,
                    //     width,
                    //     height,
                    //     image_color,
                    //     polyscope::ImageOrigin::UpperLeft);
                    // surface_mesh->addScalarImageQuantity(
                    //     texture_name + "_scalar",
                    //     width,
                    //     height,
                    //     image_scalar,
                    //     polyscope::ImageOrigin::UpperLeft);

                    // if (has_alpha) {
                    //     surface_mesh->addColorAlphaImageQuantity(
                    //         texture_name + "_alpha",
                    //         width,
                    //         height,
                    //         image_color_alpha,
                    //         polyscope::ImageOrigin::UpperLeft);
                    // }
                    for (int i = 0;
                         i <
                         mesh->get_vertex_parameterization_quantities().size();
                         i++) {
                        surface_mesh->addTextureColorQuantity(
                            "vertex texture color" + std::to_string(i),
                            "vertex parameterization" + std::to_string(i),
                            width,
                            height,
                            image_color,
                            polyscope::ImageOrigin::UpperLeft);
                        surface_mesh->addTextureScalarQuantity(
                            "vertex texture scalar" + std::to_string(i),
                            "vertex parameterization" + std::to_string(i),
                            width,
                            height,
                            image_scalar,
                            polyscope::ImageOrigin::UpperLeft);
                        if (has_alpha) {
                            surface_mesh->addTextureColorQuantity(
                                "vertex texture color alpha" +
                                    std::to_string(i),
                                "vertex parameterization" + std::to_string(i),
                                width,
                                height,
                                image_color_alpha,
                                polyscope::ImageOrigin::UpperLeft);
                        }
                    }
                    for (int i = 0;
                         i < mesh->get_face_corner_parameterization_quantities()
                                 .size();
                         i++) {
                        surface_mesh->addTextureColorQuantity(
                            "face corner texture color" + std::to_string(i),
                            "face corner parameterization" + std::to_string(i),
                            width,
                            height,
                            image_color,
                            polyscope::ImageOrigin::UpperLeft);
                        surface_mesh->addTextureScalarQuantity(
                            "face corner texture scalar" + std::to_string(i),
                            "face corner parameterization" + std::to_string(i),
                            width,
                            height,
                            image_scalar,
                            polyscope::ImageOrigin::UpperLeft);
                        if (has_alpha) {
                            surface_mesh->addTextureColorQuantity(
                                "face corner texture color alpha" +
                                    std::to_string(i),
                                "face corner parameterization" +
                                    std::to_string(i),
                                width,
                                height,
                                image_color_alpha,
                                polyscope::ImageOrigin::UpperLeft);
                        }
                    }
                }
                catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }
            else {
                // TODO: Throw something
            }
        }
    }

    auto xform_component = geometry.get_component<XformComponent>();
    if (xform_component) {
        //     auto usdgeom = pxr::UsdGeomXformable ::Get(stage, sdf_path);
        // Transform
        assert(
            xform_component->translation.size() ==
            xform_component->rotation.size());

        pxr::GfMatrix4d final_transform;
        final_transform.SetIdentity();

        for (int i = 0; i < xform_component->translation.size(); ++i) {
            pxr::GfMatrix4d t;
            t.SetTranslate(xform_component->translation[i]);
            pxr::GfMatrix4d s;
            s.SetScale(xform_component->scale[i]);

            pxr::GfMatrix4d r_x;
            r_x.SetRotate(pxr::GfRotation{ { 1, 0, 0 },
                                           xform_component->rotation[i][0] });
            pxr::GfMatrix4d r_y;
            r_y.SetRotate(pxr::GfRotation{ { 0, 1, 0 },
                                           xform_component->rotation[i][1] });
            pxr::GfMatrix4d r_z;
            r_z.SetRotate(pxr::GfRotation{ { 0, 0, 1 },
                                           xform_component->rotation[i][2] });

            auto transform = r_x * r_y * r_z * s * t;
            final_transform = final_transform * transform;
        }

        glm::mat4 glm_transform;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                glm_transform[i][j] = final_transform[i][j];
            }
        }

        structure->setTransform(glm_transform);
    }
    else {
        structure->resetTransform();
    }

    return true;
}

NODE_DECLARATION_REQUIRED(write_polyscope);

NODE_DECLARATION_UI(write_polyscope);
NODE_DEF_CLOSE_SCOPE