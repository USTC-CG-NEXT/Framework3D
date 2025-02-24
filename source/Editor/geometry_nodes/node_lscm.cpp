#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <time.h>
#include <Eigen/Sparse>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(lscm)
{
    b.add_input<Geometry>("Input");

    b.add_output<Geometry>("Output");
    b.add_output<float>("Runtime");
}

NODE_EXECUTION_FUNCTION(lscm)
{
    // Get the input from params
    auto input = params.get_input<Geometry>("Input");

    // Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>()) {
        throw std::runtime_error("LSCM Parameterization: Need Geometry Input.");
        return false;
    }

    auto halfedge_mesh = operand_to_openmesh(&input);

    //Initialization
    clock_t start_time = clock();
    int n_faces = halfedge_mesh->n_faces();
    int n_vertices = halfedge_mesh->n_vertices();

    // Construct a set of new triangles
    std::vector<double> area(n_faces);
    std::vector<std::vector<Eigen::Vector2d>> edges(n_faces);

    for (auto const& face_handle : halfedge_mesh->faces()) {
        int face_idx = face_handle.idx();
        std::vector<int> vertex_idx(3);
        std::vector<double> edge_length(3);
        int i = 0;
        for (const auto& vertex_handle : face_handle.vertices())
            vertex_idx[i++] = vertex_handle.idx();

        for (int i = 0; i < 3; i++)
            edge_length[i] = (halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[(i + 1) % 3])) -
                              halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[(i + 2) % 3])))
                              .length();

        // Calculate the area of the face
        double tmp = (edge_length[0] + edge_length[1] + edge_length[2]) / 2;
        area[face_idx] = tmp;
        for (int i = 0; i < 3; i++)
            area[face_idx] *= tmp - edge_length[i];
        area[face_idx] = sqrt(area[face_idx]);

        // Record the edges of the face
        // Their indexes are related to the point indexes opposite to them 
        edges[face_idx].resize(3);
        double cos_angle = (edge_length[1] * edge_length[1] + edge_length[2] * edge_length[2] - edge_length[0] * edge_length[0]) / (2 * edge_length[1] * edge_length[2]);
        double sin_angle = sqrt(1 - cos_angle * cos_angle);
        edges[face_idx][1] << -edge_length[1] * cos_angle, -edge_length[1] * sin_angle;
        edges[face_idx][2] << edge_length[2], 0;
        edges[face_idx][0] = -edges[face_idx][1] - edges[face_idx][2];
    }

    // Two ensured points
    int fixed = 2;
    int idx1 = -1, idx2 = -1;
    double max_dist = 0;
    for (const auto& halfedge_handle : halfedge_mesh->halfedges()) {
        if (halfedge_handle.is_boundary()) {
            if (idx1 == -1)
                idx1 = halfedge_handle.from().idx();
            const auto& v1 = halfedge_mesh->point(halfedge_mesh->vertex_handle(idx1));
            const auto& v2 = halfedge_mesh->point(halfedge_handle.from());
            if ((v1 - v2).length() > max_dist) {
                max_dist = (v1 - v2).length();
                idx2 = halfedge_handle.from().idx();
            }
            const auto& v3 = halfedge_mesh->point(halfedge_handle.to());
            if ((v1 - v3).length() > max_dist) {
                max_dist = (v1 - v3).length();
                idx2 = halfedge_handle.to().idx();
            }
        }
    }

    Eigen::VectorXd r(2 * fixed);
    r(0) = halfedge_mesh->point(halfedge_mesh->vertex_handle(idx1))[0];
    r(1) = halfedge_mesh->point(halfedge_mesh->vertex_handle(idx2))[0];
    r(2) = halfedge_mesh->point(halfedge_mesh->vertex_handle(idx1))[1];
    r(3) = halfedge_mesh->point(halfedge_mesh->vertex_handle(idx2))[1];

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
    Eigen::SparseMatrix<double> B(2 * n_faces, 2 * fixed);
    Eigen::VectorXd b(2 * n_faces);
    for (const auto& face_handle : halfedge_mesh->faces()) {
        int face_idx = face_handle.idx();
        double coeff = sqrt(2 * area[face_idx]);
        int i = 0;
        for (const auto& vertex_handle : face_handle.vertices()) {
            int vertex_idx = vertex_handle.idx();
            int mat_idx = ori2mat[vertex_idx];
            double dx = edges[face_idx][i][0] / coeff;
            double dy = edges[face_idx][i][1] / coeff;
            if (mat_idx == -1) {
                // Fixed points
                if (vertex_idx == idx1) {
                    B.coeffRef(face_idx, 0) = dx;
                    B.coeffRef(face_idx + n_faces, 0) = dy;
                    B.coeffRef(face_idx, 2) = -dy;
                    B.coeffRef(face_idx + n_faces, 2) = dx;
                }
                else if (vertex_idx == idx2) {
                    B.coeffRef(face_idx, 1) = dx;
                    B.coeffRef(face_idx + n_faces, 1) = dy;
                    B.coeffRef(face_idx, 3) = -dy;
                    B.coeffRef(face_idx + n_faces, 3) = dx;
                }
            }
            else {
                // Free points
                A.coeffRef(face_idx, mat_idx) = dx;
                A.coeffRef(face_idx + n_faces, mat_idx) = dy;
                A.coeffRef(face_idx, mat_idx + n_vertices - fixed) = -dy;
                A.coeffRef(face_idx + n_faces, mat_idx + n_vertices - fixed) = dx;
            }
            i++;
        }
    }

    // Solve the least square problem
    Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>> solver;
    solver.compute(A);
    b = -B * r;
    Eigen::VectorXd u = solver.solve(b);

    for (const auto& vertex_handle : halfedge_mesh->vertices()) {
        int idx = vertex_handle.idx();
        int mat_idx = ori2mat[idx];
        if (mat_idx != -1) {
            halfedge_mesh->point(vertex_handle)[0] = u(mat_idx);
            halfedge_mesh->point(vertex_handle)[1] = u(mat_idx + n_vertices - fixed);
        }
        halfedge_mesh->point(vertex_handle)[2] = 0;
    }

    clock_t end_time = clock();

    auto geometry = openmesh_to_operand(halfedge_mesh.get());

    // Set the output of the nodes
    params.set_output("Output", std::move(*geometry));
    params.set_output("Runtime", float(end_time - start_time));
    return true;
}

NODE_DECLARATION_UI(lscm);
NODE_DEF_CLOSE_SCOPE
