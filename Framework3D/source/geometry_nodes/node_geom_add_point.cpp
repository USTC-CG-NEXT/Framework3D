#include "GCore/Components/PointsComponent.h"
#include "GCore/geom_payload.hpp"
#include "geom_node_base.h"
#include "pxr/base/tf/ostreamMethods.h"
#include "pxr/base/vt/typeHeaders.h"
NODE_DEF_OPEN_SCOPE

struct AddedPoints {
    pxr::VtArray<pxr::GfVec3f> points;

    nlohmann::json serialize()
    {
        auto val = pxr::TfStringify(points);
        nlohmann::json ret;
        ret["points"] = val;
        return ret;
    }

    void deserialize(const nlohmann::json& json)
    {
    }
};

NODE_DECLARATION_FUNCTION(geom_add_point)
{
    b.add_input<float>("Width").min(0.001).max(1).default_val(0.1f);

    b.add_output<Geometry>("Points");
}

NODE_EXECUTION_FUNCTION(geom_add_point)
{
    //auto& storage = params.get_storage<AddedPoints&>();

    //auto pick = params.get_global_payload<GeomNodeGlobalParams>().pick;
    //if (pick) {
    //    storage.points.push_back(pxr::GfVec3f(pick->point));
    //}

    //params.set_storage(storage);

    //auto width = params.get_input<float>("Width");

    //auto geometry = Geometry();
    //auto points_component = std::make_shared<PointsComponent>(&geometry);
    //geometry.attach_component(points_component);

    //pxr::VtArray widths(storage.points.size(), width);

    //points_component->set_vertices(storage.points);
    //points_component->set_width(widths);

    //params.set_output("Points", geometry);
    return true;
}



NODE_DECLARATION_UI(geom_add_point);
NODE_DEF_CLOSE_SCOPE
