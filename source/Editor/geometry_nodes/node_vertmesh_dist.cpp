#include "nodes/core/def/node_def.hpp"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/Core>
#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include <float.h>
#include <cmath>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <pxr/usd/usdGeom/mesh.h>
#include "GCore/Components.h"
#include "GCore/GOP.h"
using Vec=Eigen::Vector3d;

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(vertmesh_dist)
{
    b.add_input<Vec>("vertex");
    b.add_input<Geometry>("Geometry");
    b.add_input<double>("radius");
    b.add_input<double>("voxelSize");
    b.add_output<double>("distance");
}

NODE_EXECUTION_FUNCTION(vertmesh_dist)
{
    auto vertex = params.get_input<Vec>("vertex");
    auto geometry = params.get_input<Geometry>("Geometry");
    double radius = params.get_input<double>("radius");
    double voxelSize = params.get_input<double>("voxelSize");
    auto mesh = operand_to_openmesh(&geometry);
    auto meshcomponent = geometry.get_component<MeshComponent>();
    auto faceVertexIndices = meshcomponent->get_face_vertex_indices();
    auto faceVertexCounts = meshcomponent->get_face_vertex_counts();
    /*int n = mesh->n_vertices();*/
    auto vertices=meshcomponent->get_vertices();
    double distance = DBL_MAX;
    Eigen::MatrixXd verticesMatrix(mesh->n_vertices(),3);
    for (const auto& vh : mesh->vertices()) {
        auto point = mesh->point(vh);
        int i = vh.idx();
        verticesMatrix(i, 0) = point[0];
        verticesMatrix(i, 1) = point[1];
        verticesMatrix(i, 2) = point[2];
    }
    Eigen::RowVector3d leftBottomCorner, rightTopCorner;
    leftBottomCorner = verticesMatrix.colwise().minCoeff();
    rightTopCorner = verticesMatrix.colwise().maxCoeff();
    double one_div_voxelSize;
    one_div_voxelSize = 1.0 / voxelSize;
    Eigen::Array<double, 1, 3> range = rightTopCorner - leftBottomCorner;
    Eigen::Array<int,1,3> voxelCount =
        (range * one_div_voxelSize).ceil().template cast<int>();
    if (voxelCount.minCoeff() <= 0) {
        // cast overflow due to huge search direction
        one_div_voxelSize = 1.0 / (range.maxCoeff() * 1.01);
        voxelCount.setOnes();
    }
    int voxelCount0x1 = voxelCount[0] * voxelCount[1];
    Eigen::Array<int, 1, 3> mins, maxs;
    Eigen::Matrix<double, 1, 3> vertex_minus;
    Eigen::Matrix<double, 1, 3> vertex_plus;
    for (int i = 0; i <= 2; i++) {
        vertex_minus[i] =
            (vertex[i] - radius - leftBottomCorner[i]) * one_div_voxelSize;
        vertex_plus[i] =
            (vertex[i] + radius - leftBottomCorner[i]) * one_div_voxelSize;
    }
    /*vertex_minus = vertex_minus * one_div_voxelSize;
    vertex_plus = vertex_minus * one_div_voxelSize;*/
    //Eigen::Matrix<double,1,3> vertex_plus = vertex.array() + radius;
    /*mins = vertex_minus * one_div_voxelSize;*/
    //mins = ((vertex_minus-leftBottomCorner)* one_div_voxelSize).array().floor().template cast<int>();
    //maxs = ((vertex_plus - leftBottomCorner) * one_div_voxelSize)
    //           .array()
    //           .floor()
    //           .template cast<int>();
    for (int i = 0; i <= 2; i++) {
        mins(i) = std::max(static_cast<int>(floor(vertex_minus[i])), 0);
        maxs(i) = std::min(
            static_cast<int>(floor(vertex_plus[i])), voxelCount[i] - 1);
    }
    /*minss = mins.max(Eigen::Array<int, 1, 3>::Zero());*/
    //maxs = maxs.min(voxelCount - 1);
    std::unordered_set<int> faceID;
    faceID.clear();
    std::map<int, std::vector<int>> voxel;
    for (int svI = 0; svI < mesh->n_vertices(); ++svI) {
        int idx = 0;
        Eigen::Matrix<double, 1, 3> pos;
        pos[0] =(verticesMatrix(svI, 0) - leftBottomCorner[0]) * one_div_voxelSize;
        pos[1] =(verticesMatrix(svI, 1) - leftBottomCorner[1]) * one_div_voxelSize;
        pos[2] =(verticesMatrix(svI, 2) - leftBottomCorner[2]) * one_div_voxelSize;
        //Eigen::Matrix<double, 1, 3>pos=verticesMatrix.row(svI);//maybe bug?
        int ix = static_cast<int>(floor(pos[0]));
        int iy = static_cast<int>(floor(pos[1]));
        int iz = static_cast<int>(floor(pos[2]));
        //voxelAxisIndex =((pos- leftBottomCorner) * one_div_voxelSize).array().floor().template cast<int>();
        assert(ix >= 0 && iy >= 0 && iz >= 0 && ix < voxelCount[0] &&iy < voxelCount[1] && iz < voxelCount[2]);
        idx = ix + iy * voxelCount[0] + iz * voxelCount0x1;
        //assert(idx >= 0 && idx < voxelCount[0] * voxelCount[1] * voxelCount[2]);
        voxel[idx].push_back(svI);
        //auto it = voxel.find(idx);
        //std::vector<int> temp = it->second;
    }
    for (int iz = mins[2]; iz <= maxs[2]; ++iz) {
        int zOffset = iz * voxelCount0x1;
        for (int iy = mins[1]; iy <= maxs[1]; ++iy) {
            int yzOffset = iy * voxelCount[0] + zOffset;
            for (int ix = mins[0]; ix <= maxs[0]; ++ix) {
                const auto voxelI = voxel.find(ix + yzOffset);
                if (voxelI != voxel.end()) {
                    for (const auto& indI : voxelI->second) {
                        if (indI >= mesh->n_faces()) {
                            faceID.insert(indI - mesh->n_faces());
                        }
                    }
                }
            }
        }
    }
    for (const auto&idx : faceID) {
        /*Vec v0, v1, v2;
        double temp=0;
        Vec vec1 = v1 - v0;
        Vec vec2 = v2 - v0;
        Vec normal = vec1.cross(vec2);
        temp= std::abs(normal.dot(v0 - vertex) / normal.norm());
        distance = std::min(distance, temp);*/
        int start=0;
        for (int i = 0; i < idx; i++) {
            start += faceVertexCounts[i];
        }
        int faceVertexCount = faceVertexCounts[idx];
        std::vector<int> faceVertexIndicesForFace;
        for (int i = 0; i < faceVertexCount; i++) {
            faceVertexIndicesForFace.push_back(faceVertexIndices[start + i]);
        }
        std::vector<pxr::GfVec3f> faceVertices;
        for(int index:faceVertexIndicesForFace){
            faceVertices.push_back(vertices[index]);
        }
        auto vertices0 = faceVertices[0];
        auto vertices1 = faceVertices[1];
        auto vertices2 = faceVertices[2];
        Vec v0(vertices0[0],vertices0[1],vertices0[2]);
        Vec v1(vertices1[0],vertices1[1],vertices1[2]);
        Vec v2(vertices2[0],vertices2[1],vertices2[2]);
        Vec vec1 = v1 - v0;
        Vec vec2 = v2 - v0;
        Vec normal = vec1.cross(vec2);
        double temp = std::abs(normal.dot(v0 - vertex) / normal.norm());
        distance = std::min(distance, temp);
    }

    params.set_output<double>("distance", std::move(distance));
    return true;
}
NODE_DECLARATION_UI(vertmesh_dist);
NODE_DEF_CLOSE_SCOPE