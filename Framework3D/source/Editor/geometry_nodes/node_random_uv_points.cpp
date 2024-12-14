#include <random>

#include "GCore/Components/PointsComponent.h"
#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(random_uv_points)
{
    b.add_input<int>("Count").min(1).max(100).default_val(10);
    b.add_input<int>("Seed").min(-100).max(100).default_val(0);
    b.add_input<float>("Width").min(0.001).max(1).default_val(0.1f);
    b.add_output<Geometry>("Points");
}

NODE_EXECUTION_FUNCTION(random_uv_points)
{
    using namespace pxr;
    auto count = params.get_input<int>("Count");
    auto seed = params.get_input<int>("Seed");
    auto width = params.get_input<float>("Width");

    std::default_random_engine engine(seed);

    auto geometry = Geometry();
    auto points_component = std::make_shared<PointsComponent>(&geometry);
    geometry.attach_component(points_component);

    pxr::VtArray<pxr::GfVec3f> points;
    pxr::VtArray<float> widths;

    std::uniform_real_distribution<float> distX(
        0.f, 1.0f - std::numeric_limits<float>::epsilon());
    std::uniform_real_distribution<float> distY(
        0.f, 1.0f - std::numeric_limits<float>::epsilon());

    for (int i = 0; i < count; ++i) {
        float x = distX(engine);
        float y = distY(engine);
        points.push_back(GfVec3f(x, y, 0.0f));
        widths.push_back(width);
    }

    points_component->set_vertices(points);
    points_component->set_width(widths);

    params.set_output("Points", geometry);
    return true;
}

NODE_DECLARATION_UI(random_uv_points);
NODE_DEF_CLOSE_SCOPE
