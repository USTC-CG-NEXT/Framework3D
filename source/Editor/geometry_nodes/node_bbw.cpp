#include <Eigen/Eigen>
#include "nodes/core/def/node_def.hpp"
#include <igl/biharmonic_coordinates.h>
#include <igl/triangle/triangulate.h>
#include <igl/barycentric_coordinates.h>

std::function<Eigen::MatrixXd(const Eigen::MatrixXd&)> generate_weight_function(
    const Eigen::MatrixXd& C,
    const Eigen::MatrixXi& E)
{
    return [C, E](const Eigen::MatrixXd& V) -> Eigen::MatrixXd {
        int k = C.rows();

        // Define the list of holes (no holes in this example)
        Eigen::MatrixXd H(0, 2);  // Empty matrix

        // Variables to store the output
        Eigen::MatrixXd V2;  // New vertices (after triangulation)
        Eigen::MatrixXi F2;  // Triangles (faces)

        // Set the flags to control the triangulation
        std::string flags = "pq30a0.001";
        // 'p': Constrained Delaunay triangulation
        // 'q30': Enforce a minimum angle of 30 degrees for all triangles
        // 'a0.1': Limit the maximum area of each triangle to 0.1

        // Perform triangulation
        igl::triangle::triangulate(C, E, H, flags, V2, F2);
        Eigen::MatrixXd W;

        std::vector<std::vector<int>> S;
        for (int i = 0; i < k; ++i) {
            S.push_back({ i });
        }
        bool success = igl::biharmonic_coordinates(V2, F2, S, W);

        Eigen::MatrixXd WW(V.rows(), k);

        for (int i = 0; i < V.rows(); ++i) {
            Eigen::MatrixXd L;
            int index = -1;
            for (int j = 0; j < F2.rows(); ++j) {
                igl::barycentric_coordinates(
                    V.row(i),
                    V2.row(F2(j, 0)),
                    V2.row(F2(j, 1)),
                    V2.row(F2(j, 2)),
                    L);
                if ((L.array() >= -1e-10).all()) {
                    index = j;
                    break;
                }
            }
            if (index != -1) {
                WW.row(i) = L(0, 0) * W.row(F2(index, 0)) +
                            L(0, 1) * W.row(F2(index, 1)) +
                            L(0, 2) * W.row(F2(index, 2));
            }
            else {
                WW.row(i) = Eigen::RowVectorXd::Zero(k);
            }
        }
        return WW;
    };
}

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(gbc_bbw)
{
    // Function content omitted
    b.add_input<Eigen::MatrixXd>("Control Points");
    b.add_output<std::function<Eigen::MatrixXd(const Eigen::MatrixXd&)>>("BBW");
}

NODE_EXECUTION_FUNCTION(gbc_bbw)
{
    // Function content omitted
    auto C = params.get_input<Eigen::MatrixXd>("Control Points");

    int num_rows = C.rows();

    Eigen::MatrixXi E(num_rows, 2);
    for (int i = 0; i < num_rows; ++i) {
        E(i, 0) = i;
        E(i, 1) = (i + 1) % num_rows;
    }

    // at least three control points
    assert(C.rows() > 2);

    // dimension of points
    assert(C.cols() == 2);

    auto W = generate_weight_function(C, E);
    // we've made W transpose
    params.set_output("BBW", W);
    return true;
}

NODE_DECLARATION_UI(gbc_bbw);
NODE_DEF_CLOSE_SCOPE
