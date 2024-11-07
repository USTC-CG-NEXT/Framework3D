#include "GCore/Components/MeshOperand.h"
#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(mesh_compose)
{
    b.add_input<pxr::VtVec3fArray>("Vertices");
    b.add_input<pxr::VtArray<int>>("FaceVertexCounts");
    b.add_input<pxr::VtArray<int>>("FaceVertexIndices");
    b.add_input<pxr::VtArray<pxr::GfVec3f>>("Normals");
    b.add_input<pxr::GfVec2f>("Texcoords");

    b.add_output<Geometry>("Mesh");
}

NODE_EXECUTION_FUNCTION(mesh_compose)
{
    Geometry geometry;
    auto mesh_component = std::make_shared<MeshComponent>(&geometry);

    auto vertices = params.get_input<pxr::VtVec3fArray>("Vertices");
    auto faceVertexCounts =
        params.get_input<pxr::VtArray<int>>("FaceVertexCounts");
    auto faceVertexIndices =
        params.get_input<pxr::VtArray<int>>("FaceVertexIndices");
    auto normals = params.get_input<pxr::VtArray<pxr::GfVec3f>>("Normals");
    auto texcoordsArray =
        params.get_input<pxr::VtArray<pxr::GfVec2f>>("Texcoords");

    if (vertices.size() > 0 && faceVertexCounts.size() > 0 &&
        faceVertexIndices.size() > 0) {
        mesh_component->set_vertices(vertices);
        mesh_component->set_face_vertex_counts(faceVertexCounts);
        mesh_component->set_face_vertex_indices(faceVertexIndices);
        mesh_component->set_normals(normals);
        mesh_component->set_texcoords_array(texcoordsArray);
        geometry.attach_component(mesh_component);
    }
    else {
        // TODO: Throw something
    }

    params.set_output("Mesh", geometry);
    return true;
}

NODE_DECLARATION_UI(mesh_compose);
NODE_DEF_CLOSE_SCOPE
