#include <exception>

#include "nodes/core/def/node_def.hpp"
#include "polyscope/curve_network.h"
#include "polyscope/pick.h"
#include "polyscope/point_cloud.h"
#include "polyscope/polyscope.h"
#include "polyscope/structure.h"
#include "polyscope/surface_mesh.h"
#include "polyscope_widget/polyscope_renderer.h"

struct PickedVerticesStorage {
    constexpr static bool has_storage = false;

    std::list<size_t> picked_vertex_indices;

};


NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(get_vertices_by_left_click)
{

    b.add_output<std::list<size_t>>("Picked Vertex Index (Left Click)");

}

NODE_EXECUTION_FUNCTION(get_vertices_by_left_click)
{
    auto pick = PolyscopeRenderer::GetPickResult();
    polyscope::Structure* structure = pick[1].first;
    size_t index = pick[1].second;

    auto& storage = params.get_storage<PickedVerticesStorage&>();

    if (structure) {
        if (structure->typeName() == "Surface Mesh") {
            auto mesh = dynamic_cast<polyscope::SurfaceMesh*>(structure);
            if (index < mesh->nVertices()) {
                storage.picked_vertex_indices.push_back(index);
            }
        }
    }
    
	std::list<size_t> picked_vertex_indices = storage.picked_vertex_indices;

	params.set_output("Picked Vertex Index (Left Click)", std::move(picked_vertex_indices));
    return true;
}

NODE_DECLARATION_UI(get_vertices_by_left_click);
NODE_DEF_CLOSE_SCOPE
