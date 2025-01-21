
#include <glintify/mesh.hpp>

USTC_CG::Mesh USTC_CG::Mesh::load_from_obj(const std::string& filename)
{
    Mesh mesh;
    OpenMesh::IO::Options opt;
    opt += OpenMesh::IO::Options::VertexNormal;
    opt += OpenMesh::IO::Options::FaceNormal;
    opt += OpenMesh::IO::Options::VertexTexCoord;

    if (!OpenMesh::IO::read_mesh(mesh.omesh, filename, opt)) {
        throw std::runtime_error("Failed to load obj file");
    }

    if (!mesh.omesh.has_face_normals()) {
        mesh.omesh.request_face_normals();
        mesh.omesh.update_face_normals();
    }
    if (!mesh.omesh.has_vertex_normals()) {
        mesh.omesh.request_vertex_normals();
        mesh.omesh.update_vertex_normals();
    }
    mesh.refresh();

    return mesh;
}

std::vector<glm::vec3> USTC_CG::Mesh::sample_on_edges(float distance)
{
    std::vector<glm::vec3> edge_samples;
    std::cout << "sampling from " << omesh.n_edges() << " of edges"
              << std::endl;
    // Assuming the mesh is already loaded into omesh
    for (auto e_it = omesh.edges_begin(); e_it != omesh.edges_end(); ++e_it) {
        auto heh = omesh.halfedge_handle(*e_it, 0);
        auto from = omesh.point(omesh.from_vertex_handle(heh));
        auto to = omesh.point(omesh.to_vertex_handle(heh));

        glm::vec3 from_vec(from[0], from[1], from[2]);
        glm::vec3 to_vec(to[0], to[1], to[2]);

        float edge_length = glm::distance(from_vec, to_vec);
        int num_samples = static_cast<int>(edge_length / distance);

        for (int i = 0; i <= num_samples; ++i) {
            float t = static_cast<float>(i) / num_samples;
            if (num_samples == 0) {
                t = 0.5f;
            }
            glm::vec3 sample_point = glm::mix(from_vec, to_vec, t);

            auto normal = omesh.normal(omesh.from_vertex_handle(heh));
            glm::vec3 nor(normal[0], normal[1], normal[2]);

            edge_samples.push_back(sample_point + 0.01f * nor);
        }
    }

    return edge_samples;
}

std::vector<glm::vec3> USTC_CG::Mesh::sample_uniform(float density)
{
    return {};
}

void USTC_CG::Mesh::refresh()
{
    vertices.clear();
    normals.clear();
    texcoords.clear();
    indices.clear();

    for (auto v_it = omesh.vertices_begin(); v_it != omesh.vertices_end();
         ++v_it) {
        auto v = omesh.point(*v_it);
        vertices.push_back({ v[0], v[1], v[2] });
    }

    // write to normals
    for (auto v_it = omesh.vertices_begin(); v_it != omesh.vertices_end();
         ++v_it) {
        if (omesh.has_vertex_normals()) {
            auto n = omesh.normal(*v_it);
            normals.push_back({ n[0], n[1], n[2] });
        }
    }

    // write to texcoords
    for (auto v_it = omesh.vertices_begin(); v_it != omesh.vertices_end();
         ++v_it) {
        if (omesh.has_vertex_texcoords2D()) {
            auto t = omesh.texcoord2D(*v_it);
            texcoords.push_back({ t[0], t[1] });
        }
    }

    // write to indices
    for (auto f_it = omesh.faces_begin(); f_it != omesh.faces_end(); ++f_it) {
        for (auto fv_it = omesh.fv_iter(*f_it); fv_it.is_valid(); ++fv_it) {
            indices.push_back(fv_it->idx());
        }
    }
}

USTC_CG::Mesh USTC_CG::Mesh::get_triangulated_mesh()
{
    auto new_mesh = *this;
    new_mesh.omesh.triangulate();
    new_mesh.refresh();
    return new_mesh;
}
