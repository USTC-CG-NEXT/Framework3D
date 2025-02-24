#include <igl/triangle/triangulate.h>
#include <pxr/base/vt/types.h>

#include <Eigen/Core>
#include <functional>

#include "GCore/geom_payload.hpp"
#include "nodes/core/def/node_def.hpp"
#include "polyscope/surface_mesh.h"
#include "pxr/base/gf/vec2f.h"
#include "pxr/base/vt/array.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(visualize_2d_function)
{
    // 输入一组二维点，按顺序连接为多边形
    b.add_input<pxr::VtArray<pxr::GfVec2f>>("2d_vertex");
    // 输入一个二维函数，返回一个标量
    b.add_input<std::function<float(float, float)>>("function");
    // 三角形最大面积的倒数
    b.add_input<int>("fineness").min(100).max(1000).default_val(100);
}

NODE_EXECUTION_FUNCTION(visualize_2d_function)
{
    auto global_payload = params.get_global_payload<GeomPayload>();
    auto stage = global_payload.stage;
    auto sdf_path = global_payload.prim_path;

    // 获取输入的二维顶点
    auto vertices = params.get_input<pxr::VtArray<pxr::GfVec2f>>("2d_vertex");

    // 获取输入的二维函数
    auto function =
        params.get_input<std::function<float(float, float)>>("function");

    // 获取三角形最大面积的倒数
    auto fineness = params.get_input<int>("fineness");

    // 将pxr::VtArray<pxr::GfVec2f>转换为Eigen矩阵
    Eigen::MatrixXd V(vertices.size(), 2);
    for (size_t i = 0; i < vertices.size(); ++i) {
        V(i, 0) = vertices[i][0];
        V(i, 1) = vertices[i][1];
    }

    // 创建边界顶点索引
    Eigen::MatrixXi E(vertices.size(), 2);
    for (size_t i = 0; i < vertices.size(); ++i) {
        E(i, 0) = i;
        E(i, 1) = (i + 1) % vertices.size();
    }

    // 使用libigl的triangulate函数生成三角形网格
    Eigen::MatrixXd V2;
    Eigen::MatrixXi F;
    std::string flags = "a" + std::to_string(1.0 / fineness) + "q";
    igl::triangle::triangulate(V, E, Eigen::MatrixXd(0, 2), flags, V2, F);

    // 将生成的三角形网格添加到polyscope中进行可视化
    std::vector<std::array<size_t, 3>> faceVertexIndicesNested;
    for (int i = 0; i < F.rows(); ++i) {
        faceVertexIndicesNested.push_back({ static_cast<size_t>(F(i, 0)),
                                            static_cast<size_t>(F(i, 1)),
                                            static_cast<size_t>(F(i, 2)) });
    }

    auto surface_mesh = polyscope::registerSurfaceMesh2D(
        sdf_path.GetName(), V2, faceVertexIndicesNested);

    // 添加顶点标量
    std::vector<float> vertex_scalar(V2.rows());
    for (int i = 0; i < V2.rows(); ++i) {
        vertex_scalar[i] = function(V2(i, 0), V2(i, 1));
    }
    surface_mesh->addVertexScalarQuantity("function", vertex_scalar);
    return true;
}

NODE_DECLARATION_REQUIRED(visualize_2d_function)

NODE_DECLARATION_UI(visualize_2d_function);
NODE_DEF_CLOSE_SCOPE