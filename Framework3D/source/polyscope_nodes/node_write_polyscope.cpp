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
#include "polyscope/point_cloud.h"
#include "polyscope/structure.h"
#include "polyscope/surface_mesh.h"
#include "polyscope_widget/polyscope_widget.h"
#include "pxr/base/gf/rotation.h"

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
// The current implementation is not been fully tested yet
NODE_EXECUTION_FUNCTION(write_polyscope)
{
    // auto global_payload = params.get_global_payload<GeomPayload>();

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

    // auto stage = global_payload.stage;
    // auto sdf_path = global_payload.prim_path;

    polyscope::Structure* structure = nullptr;

    if (mesh) {
        auto vertices = mesh->get_vertices();
        // faceVertexIndices是一个一维数组，每faceVertexCounts[i]个元素表示一个面
        auto faceVertexCounts = mesh->get_face_vertex_counts();
        auto faceVertexIndices = mesh->get_face_vertex_indices();
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

        auto mesh = polyscope::registerSurfaceMesh(
            "mesh", vertices, faceVertexIndicesNested);

        structure = mesh;
    }
    else if (points) {
        auto vertices = points->get_vertices();
        auto point_cloud = polyscope::registerPointCloud("points", vertices);

        if (points->get_width().size() > 0) {
            auto q =
                point_cloud->addScalarQuantity("width", points->get_width());
            point_cloud->setPointRadiusQuantity(q);
        }

        if (points->get_display_color().size() > 0) {
            point_cloud->addColorQuantity("color", points->get_display_color());
        }

        structure = point_cloud;
    }
    else if (curve) {
        auto vertices = curve->get_vertices();
        // vert_count是一个一维数组，每个元素表示一个curve的点数，vertices中每vert_count[i]个元素表示一个curve
        auto vert_count = curve->get_vert_count();
        // 转换为edge array
        std::vector<std::array<size_t, 2>> edges;
        size_t start = 0;
        for (size_t i = 0; i < vert_count.size(); i++) {
            for (size_t j = 0; j < vert_count[i] - 1; j++) {
                edges.push_back({ start + j, start + j + 1 });
            }
            start += vert_count[i];
        }

        auto curve = polyscope::registerCurveNetwork("curve", vertices, edges);

        structure = curve;
    }

    // Material and Texture
    auto material_component = geometry.get_component<MaterialComponent>();
    if (material_component) {
        // auto usdgeom = pxr::UsdGeomXformable ::Get(stage, sdf_path);
        if (legal(std::string(material_component->textures[0].c_str()))) {
            auto texture_name =
                std::string(material_component->textures[0].c_str());
            std::filesystem::path p =
                std::filesystem::path(texture_name).replace_extension();
            auto file_name = "texture" + p.filename().string();

            std::string material_path_root = "/TexModel";
            std::string material_path =
                material_path_root + "/" + file_name + "Mat";
            std::string material_shader_path = material_path + "/PBRShader";
            std::string material_stReader_path = material_path + "/stReader";
            std::string material_texture_path =
                material_path + "/diffuseTexture";
        }
        else {
            // TODO: Throw something
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