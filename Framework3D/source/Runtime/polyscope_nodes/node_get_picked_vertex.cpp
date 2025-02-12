#include <exception>

#include "nodes/core/def/node_def.hpp"
#include "polyscope/curve_network.h"
#include "polyscope/pick.h"
#include "polyscope/point_cloud.h"
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
    else if (structure->typeName() == "Point Cloud") {
        auto point_cloud = dynamic_cast<polyscope::PointCloud*>(structure);
        if (index < point_cloud->nPoints()) {
            params.set_output("Picked Structure Name", structure->name);
            params.set_output("Picked Vertex Index", index);

            auto pos = point_cloud->getPointPosition(index);
            params.set_output("Picked Vertex Position X", pos.x);
            params.set_output("Picked Vertex Position Y", pos.y);
            params.set_output("Picked Vertex Position Z", pos.z);
        }
        else {
            std::cerr << "The picked index is not a vertex index." << std::endl;
            return false;
        }
    }
    else if (structure->typeName() == "Curve Network") {
        auto curve_network = dynamic_cast<polyscope::CurveNetwork*>(structure);
        if (index < curve_network->nNodes()) {
            params.set_output("Picked Structure Name", structure->name);
            params.set_output("Picked Vertex Index", index);

            auto pos = curve_network->nodePositions.getValue(index);
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
        std::cerr << "The picked structure is not a surface mesh, point cloud, "
                     "or curve network."
                  << std::endl;
        return false;
    }

    return true;
}

NODE_DECLARATION_UI(get_picked_vertex);
NODE_DEF_CLOSE_SCOPE