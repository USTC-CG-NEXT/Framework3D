#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <time.h>
#include <Eigen/Sparse>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(tutte)
{
    // Function content omitted
    b.add_input<Geometry>("Input");

    b.add_output<Geometry>("Output");
    b.add_output<float>("Runtime");
}

NODE_EXECUTION_FUNCTION(tutte)
{
    // Function content omitted
    
    // Get the input from params
    auto input = params.get_input<Geometry>("Input");

    // Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>()) {
        throw std::runtime_error("Tutte Parameterization: Need Geometry Input.");
        return false;
    }

    auto halfedge_mesh = operand_to_openmesh(&input);

    // Initialization
    clock_t start_time = clock();
    int n_vertices = halfedge_mesh->n_vertices();
    std::vector<int> ori2mat(n_vertices, 0);

    // Label the boundary vertecies
    for (const auto& halfedge_handle : halfedge_mesh->halfedges())
        if (halfedge_handle.is_boundary()) {
            ori2mat[halfedge_handle.to().idx()] = -1;
            ori2mat[halfedge_handle.from().idx()] = -1;
        }

    // Construct a dictionary of internal points
    int n_internals = 0;
    for (int i = 0; i < n_vertices; i++)
        if (ori2mat[i] != -1)
            ori2mat[i] = n_internals++;

    Eigen::SparseMatrix<double> A(n_internals, n_internals);
    Eigen::VectorXd bx(n_internals);
    Eigen::VectorXd by(n_internals);
    Eigen::VectorXd bz(n_internals);

    // Construct coefficient matrix and vector
    for (const auto& vertex_handle : halfedge_mesh->vertices()) {
        int mat_idx = ori2mat[vertex_handle.idx()];
        if (mat_idx == -1)
            continue;
        bx(mat_idx) = 0;
        by(mat_idx) = 0;
        bz(mat_idx) = 0;

        int Aii = 0;
        for (const auto& halfedge_handle : vertex_handle.outgoing_halfedges()) {
            const auto& v1 = halfedge_handle.to();
            int mat_idx1 = ori2mat[v1.idx()];
            Aii++;
            if (mat_idx1 == -1) {
                // Boundary points
                bx(mat_idx) += halfedge_mesh->point(v1)[0];
                by(mat_idx) += halfedge_mesh->point(v1)[1];
                bz(mat_idx) += halfedge_mesh->point(v1)[2];
            }
            else
                // Internal points
                A.coeffRef(mat_idx, mat_idx1) = -1;
        }
        A.coeffRef(mat_idx, mat_idx) = Aii;
    }

    Eigen::SparseLU<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> solver;
    solver.compute(A);
    Eigen::VectorXd ux = bx;
    ux = solver.solve(ux);
    Eigen::VectorXd uy = by;
    uy = solver.solve(uy);
    Eigen::VectorXd uz = bz;
    uz = solver.solve(uz);

    // Update new positions
    for (const auto& vertex_handle : halfedge_mesh->vertices()) {
        int idx = ori2mat[vertex_handle.idx()];
        if (idx != -1) {
            halfedge_mesh->point(vertex_handle)[0] = ux(idx);
            halfedge_mesh->point(vertex_handle)[1] = uy(idx);
            halfedge_mesh->point(vertex_handle)[2] = uz(idx);
        }
    }

    clock_t end_time = clock();

    auto geometry = openmesh_to_operand(halfedge_mesh.get());

    // Set the output of the nodes
    params.set_output("Output", std::move(*geometry));
    params.set_output("Runtime", float(end_time - start_time));
    return true;
}

NODE_DECLARATION_UI(tutte);
NODE_DEF_CLOSE_SCOPE
