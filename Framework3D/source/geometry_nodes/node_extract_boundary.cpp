#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <Eigen/Sparse>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(extract_boundary)
{
    b.add_input<Geometry>("Input");

    b.add_output<Eigen::VectorXi>("Output");
}

NODE_EXECUTION_FUNCTION(extract_boundary)
{
    // Get the input from params
    auto input = params.get_input<Geometry>("Input");

    // Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>()) {
        throw std::runtime_error("Boundary Extraction: Need Geometry Input.");
        return false;
    }

    auto halfedge_mesh = operand_to_openmesh(&input);
    
    // Extract all boundarys
    int n_vertices = halfedge_mesh->n_vertices();
    std::vector<std::vector<int>> boundarys;
    std::vector<int> boundary_edges(n_vertices, -1);
    int n_boundarys = 0;
    for (const auto& halfedge_handle : halfedge_mesh->halfedges())
        if (halfedge_handle.is_boundary()) {
            if (boundary_edges[halfedge_handle.from().idx()] != -1)
                std::cout << "Something goes wrong!\n";
            boundary_edges[halfedge_handle.from().idx()] = halfedge_handle.to().idx();
            n_boundarys++;
        }

    int cnt = 0;
    int boundary_cnt = 0;
    int pointer = 0;
    std::vector<bool> boundary_visited(n_vertices, false);
    while (cnt < n_boundarys) {
        // Find the first index of the boundary loop
        for (; pointer < n_vertices; pointer++)
            if (boundary_edges[pointer] >= 0 && !boundary_visited[pointer])
                break;

        boundarys.push_back({pointer});
        boundary_visited[pointer] = true;
        int tmp = pointer;
        do {
            tmp = boundary_edges[tmp];
            boundarys[boundary_cnt].push_back(tmp);
            boundary_visited[tmp] = true;
            cnt++;
        } while (tmp != pointer && cnt < n_boundarys);
        boundary_cnt++;
    }

    // Extract the longest boundary
    std::sort(boundarys.begin(), boundarys.end(), [](auto& a, auto& b) {return a.size() > b.size();});
    
    // The last integer of a boundary is the same as the first
    Eigen::VectorXi boundary(boundarys[0].size());
    for (int i = 0; i < boundarys[0].size(); i++)
        boundary(i) = boundarys[0][i];

    params.set_output("Output", boundary);
    return true;
}

NODE_DECLARATION_UI(extract_boundary);
NODE_DEF_CLOSE_SCOPE
