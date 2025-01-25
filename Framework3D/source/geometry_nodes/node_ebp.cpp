#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <time.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(node_ebp)
{
    b.add_input<Geometry>("Input");
    b.add_input<Geometry>("Initialization");
    b.add_input<Eigen::VectorXi>("Boundary");
    b.add_input<float>("Shell Radius Proportion").min(1).max(2).default_val(1.05);
    b.add_input<int>("Maximum Shell Points").min(100).max(500).default_val(200);

    b.add_output<Geometry>("Output");
    b.add_output<float>("Runtime");
}

NODE_EXECUTION_FUNCTION(node_ebp)
{
    // Get the input from params
    auto input = params.get_input<Geometry>("Input");
    auto iters = params.get_input<Geometry>("Initialization");
    Eigen::VectorXi boundary = params.get_input<Eigen::VectorXi>("Boundary");
    float radius_proportion = params.get_input<float>("Shell Radius Proportion");
    int max_shell_points = params.get_input<int>("Maximum Shell Points");

    // Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>() || !iters.get_component<MeshComponent>()) {
        throw std::runtime_error("EBP: Need Geometry Input.");
    }

    // Initialization
    clock_t start_time = clock();
    auto halfedge_mesh = operand_to_openmesh(&input);
    auto iter_mesh = operand_to_openmesh(&iters);
    int n_faces = halfedge_mesh->n_faces();
    int n_vertices = halfedge_mesh->n_vertices();
    int n_boundary = boundary.rows() - 1;
    int n_shell = boundary.rows() / 10;
    if (n_shell > max_shell_points)
        n_shell = max_shell_points;

    // Form the shell out of the boundary of the initial parameterization
    Eigen::MatrixXd shell_vertices(n_shell, 2);

    // Find the radius of the original mesh
    float radius = 0;
    for (const auto& vertex_handle : iter_mesh->vertices()) {
        const auto& v = iter_mesh->point(vertex_handle);
        if (radius > v.length())
            radius = v.length();
    }
    radius *= radius_proportion;

    #define PI 3.1415926
    for (int i = 0; i < n_shell; i++) {
        double angle = 2 * PI * i / n_shell;
        shell_vertices(i, 0) = radius * cos(angle);
        shell_vertices(i, 1) = radius * sin(angle);
    }

    // Triangulate the mesh, the index of the added points are labeled after the original mesh
    Eigen::MatrixXi shell_edges(n_shell + n_boundary, 2);
    Eigen::MatrixXi shell_faces(n_shell + n_boundary, 3);
    for (int i = 0; i < n_boundary; i++) {
        shell_faces(i, 0) = boundary(i);
        shell_faces(i, 1) = boundary(i + 1);
    }

    // Calculate the angle of all boundary vertex
    Eigen::VectorXd boundary_angle(n_boundary + 1);
    for (int i = 1; i <= n_boundary; i++) {
        const auto& p0 = halfedge_mesh->point(halfedge_mesh->vertex_handle(boundary(i - 1)));
        const auto& p1 = halfedge_mesh->point(halfedge_mesh->vertex_handle(boundary(i)));
        boundary_angle(i) = (p0 - p1).norm() + boundary_angle(i - 1);
    }
    // We compare the boundary_angles with shell angles(2 * PI * shell_index / n_shell), so the constant 2 * PI is omitted
    for (int i = 1; i <= n_boundary; i++)
        boundary_angle(i) /= boundary_angle(n_boundary);

    // TODO: there can be exeptions to this triangulation algorithm, maybe use some delaunay triangulation instead
    int shell_index = 0;
    double shell_angle = 0;
    int edge_index = 0;
    int face_index = 0;
    // Points of the shell is less than the boundary, so connect all boundary points to shell points can finish the job
    for (int boundary_index = 0; boundary_index < n_boundary; boundary_index++) {
        // Shell angles are easy to calculate based on the index of the vertex
        if (abs(shell_angle - boundary_angle(boundary_index + 1)) < 0.5 / n_shell) {
            shell_faces(boundary_index, 2) = shell_index + n_vertices;
        }
        else {
            // Add another edge in the middle, ending up in adding two faces
            if (abs(shell_angle + 0.5 / n_shell - boundary(boundary_index + 1)) <
                abs(shell_angle + 0.5 / n_shell - boundary(boundary_index))) {
                shell_faces(boundary_index, 2) = shell_index + n_vertices;
                shell_faces(shell_index + n_boundary, 0) = boundary(boundary_index + 1);
                shell_faces(shell_index + n_boundary, 1) = shell_index + n_vertices;
                shell_faces(shell_index + n_boundary, 2) = (shell_index + 1) % n_shell + n_vertices;
            }
            else {
                shell_faces(boundary_index, 2) = (shell_index + 1) % n_shell + n_vertices;
                shell_faces(shell_index + n_boundary, 0) = boundary(boundary_index);
                shell_faces(shell_index + n_boundary, 1) = shell_index + n_vertices;
                shell_faces(shell_index + n_boundary, 2) = (shell_index + 1) % n_shell + n_vertices;
            }
            shell_index++;
            shell_angle += 1.0 / n_shell;
            if (shell_index > n_shell)
                std::cout << "Something goes wrong!\n";
        }
    }

    // Add up the energy of both original mesh and shelled mesh
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

    clock_t end_time = clock();

    auto geometry = openmesh_to_operand(halfedge_mesh.get());

    // Set the output of the nodes
    params.set_output("Output", std::move(*geometry));
    params.set_output("Runtime", float(end_time - start_time));
    return true;
}

NODE_DECLARATION_UI(node_ebp);
NODE_DEF_CLOSE_SCOPE
