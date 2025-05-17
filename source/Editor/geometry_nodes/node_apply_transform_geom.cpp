#include <memory>

#include "GCore/Components/XformComponent.h"
#include "GCore/Components/MeshOperand.h"
#include "geom_node_base.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/rotation.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(apply_transform_geom)
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

NODE_EXECUTION_FUNCTION(apply_transform_geom)
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

    pxr::GfMatrix4d final_transform;
    final_transform.SetIdentity();

    pxr::GfMatrix4d t;
    t.SetTranslate(pxr::GfVec3d(t_x, t_y, t_z));
    pxr::GfMatrix4d s;
    s.SetScale(pxr::GfVec3d(s_x, s_y, s_z));

    pxr::GfMatrix4d mat_r_x;
    mat_r_x.SetRotate(pxr::GfRotation{ { 1, 0, 0 }, r_x });
    pxr::GfMatrix4d mat_r_y;
    mat_r_y.SetRotate(pxr::GfRotation{ { 0, 1, 0 }, r_y });
    pxr::GfMatrix4d mat_r_z;
    mat_r_z.SetRotate(pxr::GfRotation{ { 0, 0, 1 }, r_z });

    auto transform = mat_r_x * mat_r_y * mat_r_z * s * t;
    final_transform = final_transform * transform;

    auto mesh = geometry.get_component<MeshComponent>();
    auto new_vertices = mesh->get_vertices();
    for (int i = 0; i < new_vertices.size(); i++) {
        auto rst = final_transform.TransformAffine(new_vertices[i]);
        new_vertices[i] = pxr::GfVec3f(rst);
    }
    mesh->set_vertices(new_vertices);
    params.set_output("Geometry", std::move(geometry));
    return true;
}

NODE_DECLARATION_UI(apply_transform_geom);
NODE_DEF_CLOSE_SCOPE
