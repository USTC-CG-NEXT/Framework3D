#include <exception>

#include "nodes/core/def/node_def.hpp"
#include "polyscope/pick.h"
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(get_picked_face)
{
    b.add_output<std::string>("Picked Structure Name");
    b.add_output<unsigned long long>("Picked Face Index");
    b.add_output<std::vector<unsigned long long>>("Picked Face Vertex Indices");
}

NODE_EXECUTION_FUNCTION(get_picked_face)
{
    if (!polyscope::pick::haveSelection()) {
        std::cerr << "Nothing is picked." << std::endl;
        return false;
    }

    auto pick = polyscope::pick::getSelection();
    auto structure = pick.first;
    auto index = pick.second;

    if (structure->typeName() == "Surface Mesh") {
        auto mesh = dynamic_cast<polyscope::SurfaceMesh*>(structure);
        if (mesh->nVertices() <= index &&
            index < mesh->nVertices() + mesh->nFaces()) {
            params.set_output("Picked Structure Name", structure->name);
            params.set_output("Picked Face Index", index - mesh->nVertices());

            auto ind = index - mesh->nVertices();
            auto start = mesh->faceIndsStart[ind];
            auto D = mesh->faceIndsStart[ind + 1] - start;
            std::vector<unsigned long long> vertices_indices;
            for (size_t j = 0; j < D; j++) {
                auto iV = mesh->faceIndsEntries[start + j];
                vertices_indices.push_back(iV);
            }
            params.set_output("Picked Face Vertex Indices", vertices_indices);
        }
        else {
            std::cerr << "The picked index is not a face index." << std::endl;
            return false;
        }
    }
    else {
        std::cerr << "The picked structure is not a surface mesh." << std::endl;
        return false;
    }

    return true;
}

NODE_DECLARATION_UI(get_picked_face);
NODE_DEF_CLOSE_SCOPE