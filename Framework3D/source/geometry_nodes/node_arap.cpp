#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <time.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>

/*
** @brief HW5_ARAP_Parameterization
**
** This file presents the basic framework of a "node", which processes inputs
** received from the left and outputs specific variables for downstream nodes to
** use.
**
** - In the first function, node_declare, you can set up the node's input and
** output variables.
**
** - The second function, node_exec is the execution part of the node, where we
** need to implement the node's functionality.
**
** - The third function generates the node's registration information, which
** eventually allows placing this node in the GUI interface.
**
** Your task is to fill in the required logic at the specified locations
** within this template, especially in node_exec.
*/

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(arap)
{
    // Input-1: Original 3D mesh with boundary
    // Maybe you need to add another input for initialization?
    b.add_input<Geometry>("Input");
    b.add_input<Geometry>("Initialization");

    /*
    ** NOTE: You can add more inputs or outputs if necessary. For example, in
    ** some cases, additional information (e.g. other mesh geometry, other
    ** parameters) is required to perform the computation.
    **
    ** Be sure that the input/outputs do not share the same name. You can add
    ** one geometry as
    **
    **                b.add_input<Geometry>("Input");
    **
    ** Or maybe you need a value buffer like:
    **
    **                b.add_input<float1Buffer>("Weights");
    */

    // Output-1: The UV coordinate of the mesh, provided by ARAP algorithm
    b.add_output<pxr::GfVec2f>("OutputUV");
    b.add_output<Geometry>("Output");
    b.add_output<float>("Runtime");
}

NODE_EXECUTION_FUNCTION(arap)
{
    // Get the input from params
    auto input = params.get_input<Geometry>("Input");
    auto iters = params.get_input<Geometry>("Initialization");

    // Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>() || !iters.get_component<MeshComponent>()) {
        throw std::runtime_error("ARAP Parameterization: Need Geometry Input.");
    }

    /* ----------------------------- Preprocess -------------------------------
    ** Create a halfedge structure (using OpenMesh) for the input mesh. The
    ** half-edge data structure is a widely used data structure in geometric
    ** processing, offering convenient operations for traversing and modifying
    ** mesh elements.
    */

    // Initialization
    clock_t start_time = clock();
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
            edge_length[i] = (halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[(i + 1) % 3])) -
                              halfedge_mesh->point(halfedge_mesh->vertex_handle(vertex_idx[(i + 2) % 3])))
                              .length();

        // Record the edges of the face
        // Their indexes are related to the point indexes opposite to them
        edges[face_idx].resize(3);
        double cos_angle = (edge_length[1] * edge_length[1] + edge_length[2] * edge_length[2] - edge_length[0] * edge_length[0]) / (2 * edge_length[1] * edge_length[2]);
        double sin_angle = sqrt(1 - cos_angle * cos_angle);
        edges[face_idx][1] << -edge_length[1] * cos_angle,
                              -edge_length[1] * sin_angle;
        edges[face_idx][2] << edge_length[2], 0;
        edges[face_idx][0] = -edges[face_idx][1] - edges[face_idx][2];

        // Calculate the cotangent values of the angles in this face
        // Their indexes are related to the edge indexes opposite to them,
        // orderly
        for (int i = 0; i < 3; i++) {
            double cos_value = edges[face_idx][i].dot(edges[face_idx][(i + 1) % 3]) /
                              (edges[face_idx][i].norm() * edges[face_idx][(i + 1) % 3].norm());
            double sin_value = sqrt(1 - cos_value * cos_value);
            cotangents.coeffRef(vertex_idx[i], vertex_idx[(i + 1) % 3]) = cos_value / sin_value;
        }
    }

    // Construct the matrix of the function, which can be precomputed before
    // iteration
    Eigen::SparseMatrix<double> A(n_vertices, n_vertices);
    for (const auto& vertex_handle : halfedge_mesh->vertices()) {
        int vertex_idx = vertex_handle.idx();
        double Aii = 0;
        for (auto adjacent_vertex = halfedge_mesh->vv_begin(vertex_handle);
             adjacent_vertex.is_valid();
             ++adjacent_vertex) {
            int adjacent_idx = (*adjacent_vertex).idx();
            double tmp_value = cotangents.coeffRef(vertex_idx, adjacent_idx) +
                               cotangents.coeffRef(adjacent_idx, vertex_idx);
            Aii += tmp_value;
            A.coeffRef(vertex_idx, adjacent_idx) = -tmp_value;
        }
        A.coeffRef(vertex_idx, vertex_idx) = Aii;
    }

    // Precompute the matrix
    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
    solver.compute(A);

    // A few changes is done here, for some more precomputation for each faces
    std::vector<Eigen::MatrixXd> b_pre(n_faces);
    std::vector<Eigen::MatrixXd> Edges(n_faces);
    std::vector<Eigen::MatrixXd> Cotangents(n_faces);
    std::vector<Eigen::MatrixXd> Jacobi_pre(n_faces);
    Eigen::MatrixXd transform_matrix(3, 3);
    transform_matrix << 1, -1, 0, 0, 1, -1, -1, 0, 1;
    for (const auto& face_handle : halfedge_mesh->faces()) {
        int face_idx = face_handle.idx();
        std::vector<int> vertex_idx(3);
        int i = 0;
        for (const auto& vertex_handle : face_handle.vertices())
            vertex_idx[i++] = vertex_handle.idx();
        Edges[face_idx].resize(3, 2);
        for (int i = 0; i < 3; i++)
            Edges[face_idx].row(i) = edges[face_idx][(i + 2) % 3];
        Cotangents[face_idx] = Eigen::MatrixXd::Identity(3, 3);
        for (int i = 0; i < 3; i++)
            Cotangents[face_idx](i, i) = cotangents.coeffRef(vertex_idx[i], vertex_idx[(i + 1) % 3]);
        Jacobi_pre[face_idx].resize(3, 2);
        Jacobi_pre[face_idx] = Cotangents[face_idx] * Edges[face_idx];
        b_pre[face_idx] = -Jacobi_pre[face_idx].transpose() * transform_matrix;
    }

    int max_iter = 400;
    int now_iter = 0;
    double err_pre = -1e9;
    double err = 1e9;
    Eigen::VectorXd bx(n_vertices);
    Eigen::VectorXd by(n_vertices);
    std::vector<Eigen::Matrix2d> Jacobi(n_faces);
    // Begin iteration
    do {
        bx.setZero();
        by.setZero();
        for (const auto& face_handle : iter_mesh->faces()) {
            int face_idx = face_handle.idx();

            // Calculate the Jacobian matrix of each faces
            std::vector<int> vertex_idx(3);
            int i = 0;
            for (const auto& vertex_handle : face_handle.vertices())
                vertex_idx[i++] = vertex_handle.idx();
            Eigen::MatrixXd U(2, 3);
            for (int i = 0; i < 3; i++) {
                const auto& v0 = iter_mesh->point(iter_mesh->vertex_handle(vertex_idx[i]));
                const auto& v1 = iter_mesh->point(iter_mesh->vertex_handle(vertex_idx[(i + 1) % 3]));
                U(0, i) = (v1 - v0)[0];
                U(1, i) = (v1 - v0)[1];
            }
            Jacobi[face_idx] = U * Jacobi_pre[face_idx];

            // Use SVD deformation to determine whether there is a flip, and set
            // the sigular values of the Jacobian matrix 1
            Eigen::JacobiSVD<Eigen::MatrixXd> svd(
                Jacobi[face_idx],
                Eigen::DecompositionOptions::ComputeThinU |
                    Eigen::DecompositionOptions::ComputeThinV);
            Eigen::MatrixXd svd_u = svd.matrixU();
            Eigen::MatrixXd svd_v = svd.matrixV();
            if (Jacobi[face_idx].determinant() < 0) {
                // If there is a flip, set the fliped sigular values 1 instead
                // of -1
                if (svd.singularValues()[0] < svd.singularValues()[1]) {
                    svd_u(0, 0) = -svd_u(0, 0);
                    svd_u(1, 0) = -svd_u(1, 0);
                }
                else {
                    svd_u(0, 1) = -svd_u(0, 1);
                    svd_u(1, 1) = -svd_u(1, 1);
                }
            }
            Jacobi[face_idx] = svd_u * svd_v.transpose();

            // Calculate bx and by by matrix multilplication
            Eigen::MatrixXd b = Jacobi[face_idx] * b_pre[face_idx];
            for (int i = 0; i < 3; i++) {
                bx(vertex_idx[i]) += b(0, i);
                by(vertex_idx[i]) += b(1, i);
            }
        }
        // Solve the linear equations
        Eigen::VectorXd ux = bx;
        ux = solver.solve(ux);
        Eigen::VectorXd uy = by;
        uy = solver.solve(uy);

        // Set the answers back to iter mesh
        for (const auto& vertex_handle : iter_mesh->vertices()) {
            int vertex_idx = vertex_handle.idx();
            iter_mesh->point(vertex_handle)[0] = ux(vertex_idx);
            iter_mesh->point(vertex_handle)[1] = uy(vertex_idx);
            iter_mesh->point(vertex_handle)[2] = 0;
        }

        // Calculate the error
        err_pre = err;
        err = 0;
        for (const auto& face_handle : iter_mesh->faces()) {
            int face_idx = face_handle.idx();
            std::vector<int> vertex_idx(3);
            int i = 0;
            for (const auto& vertex_handle : face_handle.vertices())
                vertex_idx[i++] = vertex_handle.idx();
            for (int i = 0; i < 3; i++) {
                int idx0 = vertex_idx[(i + 1) % 3];
                int idx1 = vertex_idx[(i + 2) % 3];
                const auto& tmp_edge =
                    iter_mesh->point(iter_mesh->vertex_handle(idx1)) -
                    iter_mesh->point(iter_mesh->vertex_handle(idx0));
                Eigen::Vector2d iter_edge;
                iter_edge << tmp_edge[0], tmp_edge[1];
                const auto& ori_edge = edges[face_idx][i];
                err += 0.5 * abs(cotangents.coeffRef(idx0, idx1)) *
                       (iter_edge - Jacobi[face_idx] * ori_edge).squaredNorm();
            }
        }
        now_iter++;
        //std::cout << now_iter << "\t" << err << std::endl;
    } while (now_iter < max_iter && abs(err - err_pre) > 1e-7);

    clock_t end_time = clock();

    auto geometry = openmesh_to_operand(halfedge_mesh.get());

    // Set the output of the nodes
    params.set_output("Output", std::move(*geometry));
    params.set_output("Runtime", float(end_time - start_time));
    return true;

    /* ------------- [HW5_TODO] ARAP Parameterization Implementation -----------
    ** Implement ARAP mesh parameterization to minimize local distortion.
    **
    ** Steps:
    ** 1. Initial Setup: Use a HW4 parameterization result as initial setup.
    **
    ** 2. Local Phase: For each triangle, compute local orthogonal approximation
    **    (Lt) by computing SVD of Jacobian(Jt) with fixed u.
    **
    ** 3. Global Phase: With Lt fixed, update parameter coordinates(u) by
    *solving
    **    a pre-factored global sparse linear system.
    **
    ** 4. Iteration: Repeat Steps 2 and 3 to refine parameterization.
    **
    ** Note:
    **  - Fixed points' selection is crucial for ARAP and ASAP.
    **  - Encapsulate algorithms into classes for modularity.
    */

    // The result UV coordinates
    pxr::VtArray<pxr::GfVec2f> uv_result;

    // Set the output of the node
    params.set_output("OutputUV", uv_result);
}

NODE_DECLARATION_UI(arap);
NODE_DEF_CLOSE_SCOPE
