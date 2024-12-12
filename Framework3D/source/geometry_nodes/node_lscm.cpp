//#include "nodes/core/def/node_def.hpp"
#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <numeric>
#include <Eigen/Sparse>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(node_lscm)
{
    b.add_input<Geometry>("Input");
    b.add_output<Geometry>("Output");
}

NODE_EXECUTION_FUNCTION(node_lscm)
{
    // Get the input from params
    auto input = params.get_input<Geometry>("Input");

    // (TO BE UPDATED) Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>()) {
        throw std::runtime_error("Minimal Surface: Need Geometry Input.");
    }

    auto halfedge_mesh = operand_to_openmesh(&input);

    int n_faces = halfedge_mesh->n_faces();
    int n_vertices = halfedge_mesh->n_vertices();

    // Construct a set of new triangles
    std::vector<double> area(n_faces);
    std::vector<std::vector<int>> vertex_index(n_vertices);
    std::vector<std::vector<Eigen::Vector2d>> edges(n_faces);

    for (auto const& face_handle : halfedge_mesh->faces()) {
        int face_idx = face_handle.idx();
        std::vector<int> vertex_idx(3);
        std::vector<double> edge_length(3);
        int i = 0;
        for (const auto& vertex_handle : face_handle.vertices())
            vertex_idx[i++] = vertex_handle.idx();

        edge_length[0] =
            (halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[1])) -
             halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[2])))
                .length();
        edge_length[1] =
            (halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[2])) -
             halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[0])))
                .length();
        
        edge_length[2] =
            (halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[0])) -
             halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[1])))
                .length();

        // Calculate the area of the face
        double tmp = std::accumulate(edge_length.begin(), edge_length.end(), 0.0) / 2;
        area[face_idx] = tmp;
        for (int i = 0; i < 3; i++)
            area[face_idx] *= tmp - edge_length[i];
        area[face_idx] = sqrt(area[face_idx]);

        // Record the edge of the face
        edges[face_idx].resize(3);
        double angle = acos(
            (edge_length[0] * edge_length[0] + edge_length[2] * edge_length[2] - edge_length[1] * edge_length[1]) /
            (2 * edge_length[0] * edge_length[2]));
        edges[face_idx][1] << -edge_length[1] * cos(angle), -edge_length[1] * sin(angle);
        edges[face_idx][2] << edge_length[2], 0;
        edges[face_idx][0] = -edges[face_idx][1] - edges[face_idx][2];
    }

    // Two ensured points
    int fixed = 2;
    int idx1 = 0;
    int idx2 = n_vertices / 2;
    std::vector<int> ori2mat(n_vertices, 0);
    ori2mat[idx1] = -1;
    ori2mat[idx2] = -1;

    // Other points
    int count = 0;
    for (const auto& vertex_handle : halfedge_mesh->vertices()) {
        int idx = vertex_handle.idx();
        if (ori2mat[idx] != -1)
            ori2mat[idx] = count++;
    }

    // Construct the matrix and vector of the function
    Eigen::SparseMatrix<double> A(2 * n_faces, 2 * (n_vertices - fixed));
    Eigen::VectorXd b(2 * n_faces);
    for (const auto& face_handle : halfedge_mesh->faces()) {
        int face_idx = face_handle.idx();
        double coeff = sqrt(2 * area[face_idx]);
        int i = 0;
        for (const auto& vertex_handle : face_handle.vertices()) {
            int vertex_idx = vertex_handle.idx();
            double dx = edges[face_idx][i][0] / coeff;
            double dy = edges[face_idx][i][1] / coeff;
            if (ori2mat[vertex_idx] == -1) {
                // Fixed points
                if (vertex_idx == idx2) {
                    b(face_idx) = -dx;
                    b(face_idx + n_faces) = -dy;
                }
                // The first fixed point would be set to (0, 0), so there will be no impact
            }
            else {
                // Free points
                A.coeffRef(face_idx, vertex_idx) = dx;
                A.coeffRef(face_idx + n_faces, vertex_idx + n_vertices - fixed) = dx;
                A.coeffRef(face_idx, vertex_idx + n_vertices - fixed) = -dy;
                A.coeffRef(face_idx + n_faces, vertex_idx) = dy;
            }
            i++;
        }
    }

    // Solve the least square problem
    Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>> solver;
    solver.compute(A);
    Eigen::VectorXd u = solver.solve(b);

    for (const auto& vertex_handle : halfedge_mesh->vertices()) {
        int idx = ori2mat[vertex_handle.idx()];
        if (idx != -1) {
            halfedge_mesh->point(vertex_handle)[0] = u(idx);
            halfedge_mesh->point(vertex_handle)[1] = u(idx + n_vertices - fixed);
            halfedge_mesh->point(vertex_handle)[2] = 0;
        }
        else {
            if (idx == idx1) {
                halfedge_mesh->point(vertex_handle)[0] = 0;
                halfedge_mesh->point(vertex_handle)[1] = 1;
                halfedge_mesh->point(vertex_handle)[2] = 0;
            }
            else {
                halfedge_mesh->point(vertex_handle)[0] = 0;
                halfedge_mesh->point(vertex_handle)[1] = 0;
                halfedge_mesh->point(vertex_handle)[2] = 0;
            }
        }
    }

    auto geometry = openmesh_to_operand(halfedge_mesh.get());

    // Set the output of the nodes
    params.set_output("Output", std::move(*geometry));
}

NODE_DECLARATION_UI(node_lscm);
NODE_DEF_CLOSE_SCOPE
