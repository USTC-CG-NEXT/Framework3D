
#include <Eigen/Eigen>
#include <functional>

#include "nodes/core/def/node_def.hpp"
std::function<Eigen::MatrixXd(const Eigen::MatrixXd&)> generate_weight_function(
    const Eigen::MatrixXd& C)
{
    return [C](const Eigen::MatrixXd& V) -> Eigen::MatrixXd {
        // number of polygon points
        int num = C.rows();
        Eigen::MatrixXd V1, C1;
        int i_prev, i_next;

        // check if either are 3D but really all z's are 0
        bool V_flat =
            (V.cols() == 3) && (std::sqrt((V.col(2)).dot(V.col(2))) < 1e-10);
        bool C_flat =
            (C.cols() == 3) && (std::sqrt((C.col(2)).dot(C.col(2))) < 1e-10);

        // if both are essentially 2D then ignore z-coords
        if ((C.cols() == 2 || C_flat) && (V.cols() == 2 || V_flat)) {
            // ignore z coordinate
            V1 = V.block(0, 0, V.rows(), 2);
            C1 = C.block(0, 0, C.rows(), 2);
        }
        else {
            // give dummy z coordinate to either mesh or poly
            V1 = V;
            C1 = C;

            // check that C is planar
            // average normal around poly corners

            Eigen::Vector3d n = Eigen::Vector3d::Zero();
            // take centroid as point on plane
            Eigen::Vector3d p = Eigen::Vector3d::Zero();
            for (int i = 0; i < num; ++i) {
                i_prev = (i > 0) ? (i - 1) : (num - 1);
                i_next = (i < num - 1) ? (i + 1) : 0;
                Eigen::Vector3d vnext =
                    (C1.row(i_next) - C1.row(i)).transpose();
                Eigen::Vector3d vprev =
                    (C1.row(i_prev) - C1.row(i)).transpose();
                n += vnext.cross(vprev);
                p += C1.row(i);
            }
            p /= num;
            n /= num;
            // normalize n
            n /= std::sqrt(n.dot(n));

            // check that poly is really coplanar
            for (int i = 0; i < num; ++i) {
                double dist_to_plane_C =
                    std::abs((C1.row(i) - p.transpose()).dot(n));
                assert(dist_to_plane_C < 1e-10);
            }

            // check that poly is really coplanar
            for (int i = 0; i < V1.rows(); ++i) {
                double dist_to_plane_V =
                    std::abs((V1.row(i) - p.transpose()).dot(n));

                if (dist_to_plane_V > 1e-10) {
                    std::cerr << "Distance from V to plane of C is large..."
                              << std::endl;
                }
            }

            // change of basis
            Eigen::Vector3d b1 = C1.row(1) - C1.row(0);
            Eigen::Vector3d b2 = n.cross(b1);
            // normalize basis rows
            b1 /= std::sqrt(b1.dot(b1));
            b2 /= std::sqrt(b2.dot(b2));
            n /= std::sqrt(n.dot(n));

            // transpose of the basis matrix in the m-file
            Eigen::Matrix3d basis = Eigen::Matrix3d::Zero();
            basis.col(0) = b1;
            basis.col(1) = b2;
            basis.col(2) = n;

            // change basis of rows vectors by right multiplying with inverse of
            // matrix with basis vectors as rows
            Eigen::ColPivHouseholderQR<Eigen::Matrix3d> solver =
                basis.colPivHouseholderQr();
            // Throw away coordinates in normal direction
            V1 = solver.solve(V1.transpose())
                     .transpose()
                     .block(0, 0, V1.rows(), 2);
            C1 = solver.solve(C1.transpose())
                     .transpose()
                     .block(0, 0, C1.rows(), 2);
        }

        // vectors from V to every C, where CmV(i,j,:) is the vector from domain
        // vertex j to handle i
        double EPS = 1e-10;
        Eigen::MatrixXd WW = Eigen::MatrixXd(C1.rows(), V1.rows());
        Eigen::MatrixXd dist_C_V(C1.rows(), V1.rows());
        std::vector<std::pair<int, int>> on_corner(0);
        std::vector<std::pair<int, int>> on_segment(0);
        for (int i = 0; i < C1.rows(); ++i) {
            i_prev = (i > 0) ? (i - 1) : (num - 1);
            i_next = (i < num - 1) ? (i + 1) : 0;
            // distance from each corner in C to the next corner so that
            // edge_length(i) is the distance from C(i,:) to C(i+1,:) defined
            // cyclically
            double edge_length = std::sqrt(
                (C1.row(i) - C1.row(i_next)).dot(C1.row(i) - C1.row(i_next)));
            for (int j = 0; j < V1.rows(); ++j) {
                Eigen::VectorXd v = C1.row(i) - V1.row(j);
                Eigen::VectorXd vnext = C1.row(i_next) - V1.row(j);
                Eigen::VectorXd vprev = C1.row(i_prev) - V1.row(j);
                // distance from V to every C, where dist_C_V(i,j) is the
                // distance from domain vertex j to handle i
                dist_C_V(i, j) = std::sqrt(v.dot(v));
                double dist_C_V_next = std::sqrt(vnext.dot(vnext));
                double a_prev =
                    std::atan2(vprev[1], vprev[0]) - std::atan2(v[1], v[0]);
                double a_next =
                    std::atan2(v[1], v[0]) - std::atan2(vnext[1], vnext[0]);
                // mean value coordinates
                WW(i, j) = (std::tan(a_prev / 2.0) + std::tan(a_next / 2.0)) /
                           dist_C_V(i, j);

                if (dist_C_V(i, j) < EPS)
                    on_corner.push_back(std::make_pair(j, i));
                else
                    // only in case of no-corner (no need for checking for
                    // multiple segments afterwards -- should only be on one
                    // segment (otherwise must be on a corner and we already
                    // handled that) domain vertex j is on the segment from i to
                    // i+1 if the distances from vj to pi and pi+1 are about
                    if (std::abs(
                            (dist_C_V(i, j) + dist_C_V_next) / edge_length -
                            1) < EPS)
                        on_segment.push_back(std::make_pair(j, i));
            }
        }

        // handle degenerate cases
        // snap vertices close to corners
        for (unsigned i = 0; i < on_corner.size(); ++i) {
            int vi = on_corner[i].first;
            int ci = on_corner[i].second;
            for (int ii = 0; ii < C.rows(); ++ii)
                WW(ii, vi) = (ii == ci) ? 1 : 0;
        }

        // snap vertices close to segments
        for (unsigned i = 0; i < on_segment.size(); ++i) {
            int vi = on_segment[i].first;
            int ci = on_segment[i].second;
            int ci_next = (ci < num - 1) ? (ci + 1) : 0;
            for (int ii = 0; ii < C.rows(); ++ii)
                if (ii == ci)
                    WW(ii, vi) = dist_C_V(ci_next, vi);
                else {
                    if (ii == ci_next)
                        WW(ii, vi) = dist_C_V(ci, vi);
                    else
                        WW(ii, vi) = 0;
                }
        }

        // normalize W
        for (int i = 0; i < V.rows(); ++i)
            WW.col(i) /= WW.col(i).sum();
        return WW.transpose();
    };
}

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(node_mvc)
{
    b.add_input<Eigen::MatrixXd>("Vertices");
    b.add_input<Eigen::MatrixXd>("Control Points");
    b.add_output<std::function<Eigen::MatrixXd(const Eigen::MatrixXd&)>>(
        "Weights");
}

NODE_EXECUTION_FUNCTION(node_mvc)
{
    // Function content omitted
    auto V = params.get_input<Eigen::MatrixXd>("Vertices");
    auto C = params.get_input<Eigen::MatrixXd>("Control Points");

    // at least three control points
    assert(C.rows() > 2);

    // dimension of points
    assert(C.cols() == 3 || C.cols() == 2);
    assert(V.cols() == 3 || V.cols() == 2);

    std::function<Eigen::MatrixXd(const Eigen::MatrixXd&)> W =
        generate_weight_function(C);

    // we've made W transpose
    params.set_output("Weights", W);
    return true;
}

NODE_DECLARATION_UI(node_mvc);
NODE_DEF_CLOSE_SCOPE
