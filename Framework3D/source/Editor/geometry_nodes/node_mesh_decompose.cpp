#include "GCore/Components/MeshOperand.h"
#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(mesh_decompose)
{
    b.add_input<Geometry>("Mesh");

    b.add_output<pxr::VtVec3fArray>("Vertices");
    b.add_output<pxr::VtArray<int>>("FaceVertexCounts");
    b.add_output<pxr::VtArray<int>>("FaceVertexIndices");
    b.add_output<pxr::VtVec3fArray>("Normals");
    b.add_output<pxr::GfVec2f>("Texcoords");
}

NODE_EXECUTION_FUNCTION(mesh_decompose)
{
    Geometry geometry = params.get_input<Geometry>("Mesh");
    auto mesh_component = geometry.get_component<MeshComponent>();

    if (mesh_component) {
        auto vertices = mesh_component->get_vertices();
        auto faceVertexCounts = mesh_component->get_face_vertex_counts();
        auto faceVertexIndices = mesh_component->get_face_vertex_indices();
        auto normals = mesh_component->get_normals();
        auto texcoordsArray = mesh_component->get_texcoords_array();

        params.set_output("Vertices", vertices);
        params.set_output("FaceVertexCounts", faceVertexCounts);
        params.set_output("FaceVertexIndices", faceVertexIndices);
        params.set_output("Normals", normals);
        params.set_output("Texcoords", texcoordsArray);
    }
    else {
        // TODO: Throw something
    }
    return true;
}

NODE_DECLARATION_UI(mesh_decompose);
NODE_DEF_CLOSE_SCOPE
