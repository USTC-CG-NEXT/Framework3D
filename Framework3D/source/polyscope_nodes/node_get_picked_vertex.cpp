#include <exception>

#include "nodes/core/def/node_def.hpp"
#include "polyscope/pick.h"
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(get_picked_vertex)
{
    b.add_output<std::string>("Picked Structure Name");
    b.add_output<unsigned long long>("Picked Vertex Index");
    b.add_output<float>("Picked Vertex Position X");
    b.add_output<float>("Picked Vertex Position Y");
    b.add_output<float>("Picked Vertex Position Z");
}

NODE_EXECUTION_FUNCTION(get_picked_vertex)
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
        if (index < mesh->nVertices()) {
            params.set_output("Picked Structure Name", structure->name);
            params.set_output("Picked Vertex Index", index);

            auto pos = mesh->vertexPositions.getValue(index);
            params.set_output("Picked Vertex Position X", pos.x);
            params.set_output("Picked Vertex Position Y", pos.y);
            params.set_output("Picked Vertex Position Z", pos.z);
        }
        else {
            std::cerr << "The picked index is not a vertex index." << std::endl;
            return false;
        }
    }
    else {
        std::cerr << "The picked structure is not a surface mesh." << std::endl;
        return false;
    }

    return true;
}

NODE_DECLARATION_UI(get_picked_vertex);
NODE_DEF_CLOSE_SCOPE