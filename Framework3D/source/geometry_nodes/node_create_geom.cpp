// #define __GNUC__
#include "GCore/Components/CurveComponent.h"
#include "GCore/Components/MeshOperand.h"
#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(create_grid)
{
    b.add_input<int>("resolution").min(1).max(20).default_val(2);
    b.add_input<float>("size").min(1).max(20);
    b.add_output<Geometry>("Geometry");
}

NODE_EXECUTION_FUNCTION(create_grid)
{
    int resolution = params.get_input<int>("resolution") + 1;
    float size = params.get_input<float>("size");
    Geometry geometry;
    std::shared_ptr<MeshComponent> mesh =
        std::make_shared<MeshComponent>(&geometry);
    geometry.attach_component(mesh);

    pxr::VtArray<pxr::GfVec3f> points;
    pxr::VtArray<pxr::GfVec2f> texcoord;
    pxr::VtArray<int> faceVertexIndices;
    pxr::VtArray<int> faceVertexCounts;

    for (int i = 0; i < resolution; ++i) {
        for (int j = 0; j < resolution; ++j) {
            float y = size * static_cast<float>(i) / (resolution - 1);
            float z = size * static_cast<float>(j) / (resolution - 1);

            float u = static_cast<float>(i) / (resolution - 1);
            float v = static_cast<float>(j) / (resolution - 1);
            points.push_back(pxr::GfVec3f(0, y, z));
            texcoord.push_back(pxr::GfVec2f(u, v));
        }
    }

    for (int i = 0; i < resolution - 1; ++i) {
        for (int j = 0; j < resolution - 1; ++j) {
            faceVertexCounts.push_back(4);
            faceVertexIndices.push_back(i * resolution + j);
            faceVertexIndices.push_back(i * resolution + j + 1);
            faceVertexIndices.push_back((i + 1) * resolution + j + 1);
            faceVertexIndices.push_back((i + 1) * resolution + j);
        }
    }

    mesh->set_vertices(points);
    mesh->set_face_vertex_indices(faceVertexIndices);
    mesh->set_face_vertex_counts(faceVertexCounts);
    mesh->set_texcoords_array(texcoord);

    params.set_output("Geometry", std::move(geometry));
    return true;
}

NODE_DECLARATION_FUNCTION(create_circle)
{
    b.add_input<int>("resolution").min(1).max(100).default_val(10);
    b.add_input<float>("radius").min(1).max(20);
    b.add_output<Geometry>("Circle");
}

NODE_EXECUTION_FUNCTION(create_circle)
{
    int resolution = params.get_input<int>("resolution");
    float radius = params.get_input<float>("radius");
    Geometry geometry;
    std::shared_ptr<CurveComponent> curve =
        std::make_shared<CurveComponent>(&geometry);
    geometry.attach_component(curve);

    pxr::VtArray<pxr::GfVec3f> points;

    pxr::GfVec3f center(0.0f, 0.0f, 0.0f);

    float angleStep = 2.0f * M_PI / resolution;

    for (int i = 0; i < resolution; ++i) {
        float angle = i * angleStep;
        pxr::GfVec3f point(
            radius * std::cos(angle) + center[0],
            radius * std::sin(angle) + center[1],
            center[2]);
        points.push_back(point);
    }

    curve->set_vertices(points);
    curve->set_vert_count({ resolution });

    curve->get_usd_curve().CreateWrapAttr(
        pxr::VtValue(pxr::UsdGeomTokens->periodic));

    params.set_output("Circle", std::move(geometry));
    return true;
}

NODE_DECLARATION_FUNCTION(create_spiral)
{
    b.add_input<int>("resolution").min(1).max(100).default_val(10);
    b.add_input<float>("R1").min(0.1).max(10).default_val(1);
    b.add_input<float>("R2").min(0.1).max(10).default_val(1);
    b.add_input<float>("Circle Count").min(0.1).max(10).default_val(2);
    b.add_input<float>("Height").min(0.1).max(10).default_val(1);
    b.add_output<Geometry>("Curve");
}

NODE_EXECUTION_FUNCTION(create_spiral)
{
    int resolution = params.get_input<int>("resolution");
    float R1 = params.get_input<float>("R1");
    float R2 = params.get_input<float>("R2");
    float circleCount = params.get_input<float>("Circle Count");
    float height = params.get_input<float>("Height");

    Geometry geometry;
    std::shared_ptr<CurveComponent> curve =
        std::make_shared<CurveComponent>(&geometry);
    geometry.attach_component(curve);

    pxr::VtArray<pxr::GfVec3f> points;

    float angleStep = circleCount * 2.0f * M_PI / resolution;
    float radiusIncrement = (R2 - R1) / resolution;
    float heightIncrement = height / resolution;

    for (int i = 0; i < resolution; ++i) {
        float angle = i * angleStep;
        float radius = R1 + radiusIncrement * i;
        float z = heightIncrement * i;
        pxr::GfVec3f point(
            radius * std::cos(angle), radius * std::sin(angle), z);
        points.push_back(point);
    }

    curve->set_vertices(points);
    curve->set_vert_count({ resolution });

    // Since a spiral is not periodic, we don't set a wrap attribute like we did
    // for the circle.

    params.set_output("Curve", std::move(geometry));
    return true;
}

NODE_DECLARATION_UI(create_geom);
NODE_DEF_CLOSE_SCOPE
