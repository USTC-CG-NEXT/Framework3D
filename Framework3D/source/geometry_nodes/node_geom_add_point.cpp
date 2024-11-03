#include "GCore/Components/PointsComponent.h"
#include "GCore/geom_node_global_payload.h"
#include "GUI/ui_event.h"
#include "RCore/Backend.hpp"
#include "geom_node_base.h"
#include "nvrhi/utils.h"
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

NODE_DECLARATION_FUNCTION(declare)
{
    b.add_storage<AddedPoints>();

    b.add_input<float>("Width").min(0.001).max(1).default_val(0.1f);

    b.add_output<Geometry>("Points");
}

NODE_EXECUTION_FUNCTION(exec)
{
    auto& storage = params.get_storage<AddedPoints&>();

    auto pick = params.get_global_payload<GeomNodeGlobalParams>().pick;
    if (pick) {
        storage.points.push_back(pxr::GfVec3f(pick->point));
    }

    params.set_storage(storage);

    auto width = params.get_input<float>("Width");

    auto geometry = Geometry();
    auto points_component = std::make_shared<PointsComponent>(&geometry);
    geometry.attach_component(points_component);

    pxr::VtArray widths(storage.points.size(), width);

    points_component->set_vertices(storage.points);
    points_component->set_width(widths);

    params.set_output("Points", geometry);
}



NODE_DECLARATION_UI(geom_add_point);
NODE_DEF_CLOSE_SCOPE
