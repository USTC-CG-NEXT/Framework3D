#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Sparse>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(extract_areas)
{
    // Function content omitted
    b.add_input<Geometry>("Input");

    b.add_output<Eigen::VectorXd>("Output");
}

NODE_EXECUTION_FUNCTION(extract_areas)
{
    // Function content omitted
    // Get the input from params
    auto input = params.get_input<Geometry>("Input");

    // Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>()) {
        throw std::runtime_error("Areas Extraction: Need Geometry Input.");
    }

    // Initialization
    auto halfedge_mesh = operand_to_openmesh(&input);
    int n_faces = halfedge_mesh->n_faces();
    int n_vertices = halfedge_mesh->n_vertices();


    // Construct a set of new triangles
    Eigen::VectorXd area(n_faces);
    Eigen::SparseMatrix<double> cotangents(n_vertices, n_vertices);

    for (auto const& face_handle : halfedge_mesh->faces()) {
        int face_idx = face_handle.idx();
        std::vector<int> vertex_idx(3);
        std::vector<double> edge_length(3);
        int i = 0;
        for (const auto& vertex_handle : face_handle.vertices())
            vertex_idx[i++] = vertex_handle.idx();

        for (int i = 0; i < 3; i++)
            edge_length[i] =
                (halfedge_mesh->point(
                     halfedge_mesh->vertex_handle(vertex_idx[(i + 1) % 3])) -
                 halfedge_mesh->point(
                     halfedge_mesh->vertex_handle(vertex_idx[(i + 2) % 3])))
                    .length();

        // Calculate the area of the face
        double tmp = (edge_length[0] + edge_length[1] + edge_length[2]) / 2;
        area(face_idx) = tmp;
        for (int i = 0; i < 3; i++)
            area(face_idx) *= tmp - edge_length[i];
        area(face_idx) = sqrt(area(face_idx));
    }
    
    params.set_output("Output", area);
    return true;
}

NODE_DECLARATION_UI(extract_areas);
NODE_DEF_CLOSE_SCOPE
