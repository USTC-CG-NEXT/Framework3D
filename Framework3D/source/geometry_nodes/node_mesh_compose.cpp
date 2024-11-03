#include "GCore/Components/MeshOperand.h"
#include "Nodes/GlobalUsdStage.h"
#include "Nodes/node.hpp"
#include "Nodes/node_declare.hpp"
#include "Nodes/node_register.h"
#include "geom_node_base.h"

namespace USTC_CG::node_mesh_compose {
static void node_declare(NodeDeclarationBuilder& b)
{
    b.add_input<decl::Float3Buffer>("Vertices");
    b.add_input<decl::Int1Buffer>("FaceVertexCounts");
    b.add_input<decl::Int1Buffer>("FaceVertexIndices");
    b.add_input<decl::Float3Buffer>("Normals");
    b.add_input<decl::Float2Buffer>("Texcoords");

    b.add_output<decl::Geometry>("Mesh");
}

static void node_exec(ExeParams params)
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
}

static void node_register()
{
    static NodeTypeInfo ntype;

    strcpy(ntype.ui_name, "Mesh Compose");
    strcpy(ntype.id_name, "geom_mesh_compose");

    geo_node_type_base(&ntype);
    ntype.node_execute = node_exec;
    ntype.declare = node_declare;
    nodeRegisterType(&ntype);
}


}  // namespace USTC_CG::node_mesh_compose
