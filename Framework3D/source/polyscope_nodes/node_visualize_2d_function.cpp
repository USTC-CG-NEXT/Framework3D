#include <pxr/base/vt/types.h>

#include <functional>

#include "GCore/geom_payload.hpp"
#include "OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh"
#include "OpenMesh/Tools/Subdivider/Uniform/LoopT.hh"
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
    // 细分次数
    b.add_input<int>("subdivision").default_val(1).min(0).max(6);
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

    // 获取细分次数
    auto subdivision = params.get_input<int>("subdivision");

    // 创建一个网格对象
    typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;
    MyMesh mesh;

    // 将顶点添加到网格中
    std::vector<MyMesh::VertexHandle> vhandles;
    for (const auto& vertex : vertices) {
        vhandles.push_back(
            mesh.add_vertex(MyMesh::Point(vertex[0], vertex[1], 0.0f)));
    }

    // 创建一个面
    std::vector<MyMesh::VertexHandle> face_vhandles;
    for (const auto& vh : vhandles) {
        face_vhandles.push_back(vh);
    }
    mesh.add_face(face_vhandles);

    // 使用Loop细分算法对多边形进行细分
    OpenMesh::Subdivider::Uniform::LoopT<MyMesh> loop_subdivider;
    loop_subdivider.attach(mesh);
    loop_subdivider(subdivision);
    loop_subdivider.detach();

    // 获取所有顶点的x和y坐标，以及顶点的函数值
    pxr::VtArray<pxr::GfVec2f> vertex;
    pxr::VtArray<float> vertex_scalar;
    for (auto vh : mesh.vertices()) {
        auto point = mesh.point(vh);
        vertex.push_back(pxr::GfVec2f(point[0], point[1]));
        vertex_scalar.push_back(function(point[0], point[1]));
    }

    // 获取所有面的顶点索引
    pxr::VtArray<pxr::VtArray<size_t>> faceVertexIndicesNested;
    for (auto fh : mesh.faces()) {
        pxr::VtArray<size_t> face;
        for (auto fv_it = mesh.fv_iter(fh); fv_it.is_valid(); ++fv_it) {
            face.push_back(fv_it->idx());
        }
        faceVertexIndicesNested.push_back(face);
    }

    // 创建一个SurfaceMesh对象
    auto surface_mesh = polyscope::registerSurfaceMesh(
        sdf_path.GetName(), vertex, faceVertexIndicesNested);

    // 添加顶点标量
    surface_mesh->addVertexScalarQuantity("function", vertex_scalar);
    return true;
}

NODE_DECLARATION_REQUIRED(visualize_2d_function)

NODE_DECLARATION_UI(visualize_2d_function);
NODE_DEF_CLOSE_SCOPE