#include <memory>

#include "GCore/Components/XformComponent.h"
#include "geom_node_base.h"
#include "pxr/base/gf/matrix4f.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(transform_geom)
{
    b.add_input<Geometry>("Geometry");

    b.add_input<float>("Translate X").min(-10).max(10).default_val(0);
    b.add_input<float>("Translate Y").min(-10).max(10).default_val(0);
    b.add_input<float>("Translate Z").min(-10).max(10).default_val(0);

    b.add_input<float>("Rotate X").min(-180).max(180).default_val(0);
    b.add_input<float>("Rotate Y").min(-180).max(180).default_val(0);
    b.add_input<float>("Rotate Z").min(-180).max(180).default_val(0);

    b.add_input<float>("Scale X").min(0.1f).max(10).default_val(1);
    b.add_input<float>("Scale Y").min(0.1f).max(10).default_val(1);
    b.add_input<float>("Scale Z").min(0.1f).max(10).default_val(1);

    b.add_output<Geometry>("Geometry");
}

NODE_EXECUTION_FUNCTION(transform_geom)
{
    auto geometry = params.get_input<Geometry>("Geometry");

    auto t_x = params.get_input<float>("Translate X");
    auto t_y = params.get_input<float>("Translate Y");
    auto t_z = params.get_input<float>("Translate Z");

    auto r_x = params.get_input<float>("Rotate X");
    auto r_y = params.get_input<float>("Rotate Y");
    auto r_z = params.get_input<float>("Rotate Z");

    auto s_x = params.get_input<float>("Scale X");
    auto s_y = params.get_input<float>("Scale Y");
    auto s_z = params.get_input<float>("Scale Z");

    std::shared_ptr<XformComponent> xform;
    xform = geometry.get_component<XformComponent>();
    if (!xform) {
        xform = std::make_shared<XformComponent>(&geometry);
        geometry.attach_component(xform);
    }

    xform->translation.push_back(pxr::GfVec3f(t_x, t_y, t_z));
    xform->scale.push_back(pxr::GfVec3f(s_x, s_y, s_z));
    xform->rotation.push_back(pxr::GfVec3f(r_x, r_y, r_z));

    params.set_output("Geometry", std::move(geometry));
    return true;
}

NODE_DECLARATION_UI(transform_geom);
NODE_DEF_CLOSE_SCOPE
