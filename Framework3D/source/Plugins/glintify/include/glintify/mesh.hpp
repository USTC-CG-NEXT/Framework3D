
#pragma once

#include <glintify/api.h>

#include <glm/glm.hpp>

#include "OpenMesh/Core/IO/MeshIO.hh"
#include "OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh"
#include "OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh"

USTC_CG_NAMESPACE_OPEN_SCOPE
using OpenMeshPolyMesh =
    OpenMesh::PolyMesh_ArrayKernelT<OpenMesh::DefaultTraits>;

using OpenMeshTriMesh = OpenMesh::TriMesh_ArrayKernelT<OpenMesh::DefaultTraits>;

class Mesh {
   public:
    static Mesh load_from_obj(const std::string& filename);

    std::vector<glm::vec3> sample_on_edges(float distance);
    std::vector<glm::vec3> sample_uniform(float density);

    // std::vector<glm::vec3> sample_based_on_image()

    void refresh();
    Mesh get_triangulated_mesh();

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned int> indices;

    OpenMeshPolyMesh omesh;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE