#include "GCore/Components/MeshOperand.h"
#include "GCore/Components/PointsComponent.h"
#include "GCore/GOP.h"
#include "geom_node_base.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/vt/array.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(set_vert_color)
{
    b.add_input<Geometry>("Geometry");
    b.add_input<pxr::VtVec3fArray>("Color");
    b.add_output<Geometry>("Geometry");
}

NODE_EXECUTION_FUNCTION(set_vert_color)
{
    // Left empty.
    auto color = params.get_input<pxr::VtArray<pxr::GfVec3f>>("Color");
    auto geometry = params.get_input<Geometry>("Geometry");

    auto mesh = geometry.get_component<MeshComponent>();
    auto points = geometry.get_component<PointsComponent>();
    if (mesh) {
        mesh->set_display_color(color);
    }
    else if (points) {
        points->get_display_color() = color;
    }
    else {
        throw std::runtime_error("The input is not a mesh or points");
    }

    params.set_output("Geometry", std::move(geometry));
}

NODE_DECLARATION_UI(set_vert_color);
NODE_DEF_CLOSE_SCOPE
