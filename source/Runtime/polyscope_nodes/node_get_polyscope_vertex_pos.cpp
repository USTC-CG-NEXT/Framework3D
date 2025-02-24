#include <string>

#include "nodes/core/def/node_def.hpp"
#include "polyscope/curve_network.h"
#include "polyscope/point_cloud.h"
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(get_polyscope_vertex_pos)
{
    b.add_input<std::string>("Structure Name");
    b.add_input<unsigned long long>("Vertex Index");
    b.add_output<float>("Vertex Position X");
    b.add_output<float>("Vertex Position Y");
    b.add_output<float>("Vertex Position Z");
}

NODE_EXECUTION_FUNCTION(get_polyscope_vertex_pos)
{
    auto structure_name = params.get_input<std::string>("Structure Name");
    structure_name = std::string(structure_name.c_str());
    auto vertex_index = params.get_input<unsigned long long>("Vertex Index");

    // If the input structure is a surface mesh
    if (polyscope::hasStructure("Surface Mesh", structure_name)) {
        auto structure =
            polyscope::getStructure("Surface Mesh", structure_name);
        auto mesh = dynamic_cast<polyscope::SurfaceMesh *>(structure);
        if (vertex_index < mesh->nVertices()) {
            auto pos = mesh->vertexPositions.getValue(vertex_index);
            params.set_output("Vertex Position X", pos.x);
            params.set_output("Vertex Position Y", pos.y);
            params.set_output("Vertex Position Z", pos.z);
        }
        else {
            std::cerr << "The picked index is not a vertex index." << std::endl;
            return false;
        }
    }
    // If the input structure is a point cloud
    else if (polyscope::hasStructure("Point Cloud", structure_name)) {
        auto structure = polyscope::getStructure("Point Cloud", structure_name);
        auto point_cloud = dynamic_cast<polyscope::PointCloud *>(structure);
        auto point = point_cloud->getPointPosition(vertex_index);
        params.set_output("Vertex Position X", point.x);
        params.set_output("Vertex Position Y", point.y);
        params.set_output("Vertex Position Z", point.z);
    }
    // If the input is a curve network
    else if (polyscope::hasStructure("Curve Network", structure_name)) {
        auto structure =
            polyscope::getStructure("Curve Network", structure_name);
        auto curve_network = dynamic_cast<polyscope::CurveNetwork *>(structure);
        if (vertex_index < curve_network->nNodes()) {
            auto pos = curve_network->nodePositions.getValue(vertex_index);
            params.set_output("Vertex Position X", pos.x);
            params.set_output("Vertex Position Y", pos.y);
            params.set_output("Vertex Position Z", pos.z);
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

NODE_DECLARATION_UI(get_polyscope_vertex_pos);
NODE_DEF_CLOSE_SCOPE