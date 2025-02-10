#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <time.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(ebp)
{
    b.add_input<Geometry>("Input");
    b.add_input<Geometry>("Initialization");
    b.add_input<Eigen::VectorXi>("Boundary");
    
    b.add_input<float>("Shell Radius Proportion").min(1).max(2).default_val(1.05);
    b.add_input<int>("Maximum Shell Points").min(100).max(500).default_val(200);

    b.add_output<Geometry>("Output");
    b.add_output<float>("Runtime");
}

static double ebp_energy(std::vector<Eigen::Matrix2d> origin_matrix, Eigen::MatrixXd new_points, Eigen::MatrixXi faces, Eigen::VectorXd areas, int n_shell, double epsilon, double lambda_b, double lambda_s)
{
    double result = 0;
    int n_vertices = new_points.rows() - n_shell;
    // E_d(M) + E_d(S)
    for (int i = 0; i < faces.rows(); i++) {
        Eigen::Matrix2d Jacobi;
        Jacobi.col(0) = (new_points.row(faces(i, 0)) - new_points.row(faces(i, 1))).transpose();
        Jacobi.col(1) = (new_points.row(faces(i, 0)) - new_points.row(faces(i, 1))).transpose();
        Jacobi = Jacobi * origin_matrix[i].inverse();
        if (faces(i, 0) >= n_vertices || faces(i, 1) >= n_vertices || faces(i, 2) >= n_vertices)
            result += areas(i) * lambda_s * (Jacobi.squaredNorm() + Jacobi.inverse().squaredNorm());
        else
            result += areas(i) * (Jacobi.squaredNorm() + Jacobi.inverse().squaredNorm());
    }
    result *= 0.25;

    // E_b(S)
    for (int i = n_vertices; i < new_points.rows(); i++)
        for (int j = n_vertices; j < new_points.rows(); j++) {
            int next_i = i + 1;
            if (next_i == new_points.rows())
                next_i = n_vertices;
            Eigen::Vector2d v1 = new_points.row(i) - new_points.row(next_i);
            Eigen::Vector2d v2 = new_points.row(i) - new_points.row(j);
            double dist = v1.cross(v2).norm() / v1.norm();
            if (dist < epsilon) {
                result += pow((epsilon / dist - 1), 2);
            }
        }

    return result;
}

static double mesh_energy(std::vector<Eigen::Matrix2d> origin_matrix, Eigen::MatrixXd new_points, Eigen::MatrixXi faces, Eigen::VectorXd areas, int n_shell)
{
    double result = 0;
    int n_vertices = new_points.rows() - n_shell;
    double area_sum = 0;
    for (int i = 0; i < faces.rows(); i++) {
        Eigen::Matrix2d Jacobi;
        Jacobi.col(0) = (new_points.row(faces(i, 0)) - new_points.row(faces(i, 1))).transpose();
        Jacobi.col(1) = (new_points.row(faces(i, 0)) - new_points.row(faces(i, 1))).transpose();
        Jacobi = Jacobi * origin_matrix[i].inverse();
        if (faces(i, 0) < n_vertices && faces(i, 1) < n_vertices &&
            faces(i, 2) < n_vertices) {
            result += areas(i) * (Jacobi.squaredNorm() + Jacobi.inverse().squaredNorm());
            area_sum += areas(i);
        }
    }
    result *= 0.25 / area_sum;
    return result;
}

NODE_EXECUTION_FUNCTION(ebp)
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

    // TODO: Make the following part a new node as the deformation node
    Eigen::MatrixXi faces(n_faces + n_shell + n_boundary, 3);
    for (auto const& face_handle : halfedge_mesh->faces()) {
        int tmp = 0;
        for (const auto& vertex_handle : face_handle.vertices())
            faces(face_handle.idx(), tmp++) = vertex_handle.idx();
    }
    Eigen::MatrixXd points(n_vertices + n_shell, 2);
    for (auto const& vertex_handle : iter_mesh->vertices()) {
        int vertex_idx = vertex_handle.idx();
        for (int i = 0; i < 2; i++)
            points(vertex_idx, i) = iter_mesh->point(
                iter_mesh->vertex_handle(vertex_idx))[i];
    }

    for (int i = n_faces; i < faces.rows(); i++)
        faces.row(i) = shell_faces.row(i - n_faces);
    for (int i = n_vertices; i < points.rows(); i++)
        points.row(i) = shell_vertices.row(i - n_vertices);

    // Pre-calculation with the original mesh
    std::vector<Eigen::Matrix2d> origin_matrix;
    Eigen::VectorXd areas(n_faces + n_shell + n_boundary);

    for (int face_idx = 0; face_idx < faces.rows(); face_idx++) {
        // Record the lengths of edges of the face
        // Their indexes are related to the point indexes opposite to them
        std::vector<double> edge_length(3);
        for (int i = 0; i < 3; i++)
            edge_length[i] = (halfedge_mesh->point(halfedge_mesh->vertex_handle(faces(face_idx, (i + 1) % 3))) -
                              halfedge_mesh->point(halfedge_mesh->vertex_handle(faces(face_idx, (i + 2) % 3)))).length();

        double cos_angle =
            (edge_length[1] * edge_length[1] + edge_length[2] * edge_length[2] -
             edge_length[0] * edge_length[0]) /
            (2 * edge_length[1] * edge_length[2]);
        double sin_angle = sqrt(1 - cos_angle * cos_angle);
        Eigen::Matrix2d tmp_matrix;
        tmp_matrix(0, 0) = -edge_length[2];
        tmp_matrix(1, 0) = 0;
        tmp_matrix(0, 1) = -edge_length[1] * cos_angle;
        tmp_matrix(1, 1) = -edge_length[1] * sin_angle;
        origin_matrix.push_back(tmp_matrix);
        areas(face_idx) = 0.5 * edge_length[2] * edge_length[1] * sin_angle;
    }

    // Pre-calculate the hessian matrix, with a few items remain unknown
    Eigen::SparseMatrix<double> Hessian(2 * (n_vertices + n_shell), 2 * (n_vertices + n_shell));
    std::vector<Eigen::Triplet<double>> triple;
    Eigen::MatrixXd coeff(9, 4);
    coeff <<  2,  2,  2,  2,
             -2, -1, -1,  0,
              0, -1, -1, -2,
             -2, -1, -1,  0,
              2,  0,  0,  0,
              0,  1,  1,  0,
              0, -1, -1,  2,
              0,  1,  1,  0,
              0,  0,  0,  2;
    for (int face_idx = 0; face_idx < faces.rows(); face_idx++) {
        Eigen::Matrix2d A = origin_matrix[face_idx].inverse();
        A = A * A.transpose();
        Eigen::VectorXd a(4);
        a(0) = A(0, 0);
        a(1) = A(0, 1);
        a(2) = A(1, 0);
        a(3) = A(1, 1);
        Eigen::VectorXd hessian_flatten = coeff * a;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++) {
                // H_x
                triple.push_back(Eigen::Triplet<double>(
                    faces(face_idx, i),
                    faces(face_idx, j),
                    hessian_flatten(i * 3 + j)));
                // H_y
                triple.push_back(Eigen::Triplet<double>(
                    faces(face_idx, i) + n_vertices + n_shell,
                    faces(face_idx, j) + n_vertices + n_shell,
                    hessian_flatten(i * 3 + j)));
            }
    }

    // Save some space for boundary energy
    for (int i = 0; i < n_shell; i++)
        for (int j = 0; j < n_shell; j++) {
            triple.push_back(Eigen::Triplet<double>(
                i + n_vertices, 
                j + n_vertices, 
                0));
            triple.push_back(Eigen::Triplet<double>(
                i + 2 * n_vertices + n_shell,
                j + 2 * n_vertices + n_shell,
                0));
        }

    Hessian.setFromTriplets(triple.begin(), triple.end());
    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
    solver.analyzePattern(Hessian);

    // Begin iteration
    int max_iteration = 100;
    int iter = 0;

    Eigen::VectorXd direction(n_vertices + n_shell);
    double current_energy = 0;
    double last_energy = mesh_energy(origin_matrix, points, faces, areas, n_shell);
    double lambda_s;
    while (iter < max_iteration) {
        // Calculate the gradient vector to determine the search direction
        Eigen::VectorXd grad(n_vertices + n_shell);
        // TODO: Fix the math

        // Finish the hessian matrix by re-calculating the right down corner every iteration

        // Solve the linear system
        solver.factorize(Hessian);
        direction = solver.solve(grad);

        // Perform line search based on direction computed earlier
        double t = 0;

        // make a step
        for (int i = 0; i < n_vertices; i++) {
            
        }

        iter++;
    }

    clock_t end_time = clock();

    auto geometry = openmesh_to_operand(halfedge_mesh.get());

    // Set the output of the nodes
    params.set_output("Output", std::move(*geometry));
    params.set_output("Runtime", float(end_time - start_time));
    return true;
}

NODE_DECLARATION_UI(ebp);
NODE_DEF_CLOSE_SCOPE
