#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Sparse>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(extract_singular_values)
{
    // Function content omitted
    b.add_input<Geometry>("Initial Mesh");
    b.add_input<Geometry>("Parameterization");

    b.add_output<Eigen::MatrixXd>("Output");
}

NODE_EXECUTION_FUNCTION(extract_singular_values)
{
    // Function content omitted
    // Get the input from params
    auto input = params.get_input<Geometry>("Initial Mesh");
    auto iters = params.get_input<Geometry>("Parameterization");

    // Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>() ||
        !iters.get_component<MeshComponent>()) {
        throw std::runtime_error("Singular Values Extraction: Need Geometry Input.");
    }

    // Initialization
    auto halfedge_mesh = operand_to_openmesh(&input);
    auto iter_mesh = operand_to_openmesh(&iters);
    int n_faces = halfedge_mesh->n_faces();
    int n_vertices = halfedge_mesh->n_vertices();


    // Construct a set of new triangles
    std::vector<std::vector<Eigen::Vector2d>> edges(n_faces);
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

        // Record the edges of the face
        // Their indexes are related to the point indexes opposite to them
        edges[face_idx].resize(3);
        double cos_angle =
            (edge_length[1] * edge_length[1] + edge_length[2] * edge_length[2] -
             edge_length[0] * edge_length[0]) /
            (2 * edge_length[1] * edge_length[2]);
        double sin_angle = sqrt(1 - cos_angle * cos_angle);
        edges[face_idx][1] << -edge_length[1] * cos_angle,
            -edge_length[1] * sin_angle;
        edges[face_idx][2] << edge_length[2], 0;
        edges[face_idx][0] = -edges[face_idx][1] - edges[face_idx][2];

        // Calculate the cotangent values of the angles in this face
        // Their indexes are related to the edge indexes opposite to them,
        // orderly
        for (int i = 0; i < 3; i++) {
            double cos_value =
                edges[face_idx][i].dot(edges[face_idx][(i + 1) % 3]) /
                (edges[face_idx][i].norm() *
                 edges[face_idx][(i + 1) % 3].norm());
            double sin_value = sqrt(1 - cos_value * cos_value);
            cotangents.coeffRef(vertex_idx[i], vertex_idx[(i + 1) % 3]) =
                cos_value / sin_value;
        }
    }

    // A few changes is done here, for some more precomputation for each faces
    Eigen::MatrixXd X(3, 2);
    Eigen::MatrixXd Cotangents = Eigen::Matrix3d::Zero();
    Eigen::MatrixXd U(2, 3);
    Eigen::MatrixXd Jacobi(2, 2);
    Eigen::MatrixXd sigmas(n_faces, 2);
    double energy = 0;
    for (const auto& face_handle : halfedge_mesh->faces()) {
        int face_idx = face_handle.idx();
        std::vector<int> vertex_idx(3);
        int i = 0;
        for (const auto& vertex_handle : face_handle.vertices())
            vertex_idx[i++] = vertex_handle.idx();
        for (int i = 0; i < 3; i++) {
            X.row(i) = edges[face_idx][(i + 2) % 3];
            Cotangents(i, i) =
                cotangents.coeffRef(vertex_idx[i], vertex_idx[(i + 1) % 3]);
            const auto& v0 = iter_mesh->point(iter_mesh->vertex_handle(vertex_idx[i]));
            const auto& v1 = iter_mesh->point(iter_mesh->vertex_handle(vertex_idx[(i + 1) % 3]));
            U(0, i) = (v1 - v0)[0];
            U(1, i) = (v1 - v0)[1];
        }
        Jacobi = U * Cotangents * X;

        // Use SVD deformation to determine whether there is a flip, and set
        // the sigular values of the Jacobian matrix 1
        Eigen::JacobiSVD<Eigen::MatrixXd> svd(
            Jacobi,
            Eigen::DecompositionOptions::ComputeThinU |
                Eigen::DecompositionOptions::ComputeThinV);

        sigmas(face_idx, 0) = svd.singularValues()[0];
        sigmas(face_idx, 1) = svd.singularValues()[1];
    }

    params.set_output("Output", sigmas);
    return true;
}

NODE_DECLARATION_UI(extract_singular_values);
NODE_DEF_CLOSE_SCOPE
