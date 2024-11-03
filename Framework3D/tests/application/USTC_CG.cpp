#include <gtest/gtest.h>

#include "GCore/GOP.h"
#include "GUI/window.h"
#include "Logger/Logger.h"
#include "geom_system.hpp"
#include "nodes/system/node_system.hpp"
#include "nodes/ui/imgui.hpp"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "stage/stage.hpp"
#include "usd_nodejson.hpp"
#include "widgets/usdtree/usd_fileviewer.h"
#include "widgets/usdview/usdview_widget.hpp"
using namespace USTC_CG;

int main()
{
    log::SetMinSeverity(Severity::Debug);
    log::EnableOutputToConsole(true);
    auto stage = create_global_stage();
    init(stage.get());
    // Add a sphere

    auto widget = std::make_unique<UsdFileViewer>(stage.get());
    auto render = std::make_unique<UsdviewEngine>(stage->get_usd_stage());

    auto window = std::make_unique<Window>();

    window->register_widget(std::move(widget));
    window->register_widget(std::move(render));

    window->register_function_perframe([&stage](Window* window) {
        pxr::SdfPath json_path;
        if (stage->consume_editor_creation(json_path)) {
            auto system = create_dynamic_loading_system();

            system->register_cpp_types<int>();
            auto loaded = system->load_configuration("geometry_nodes.json");
            loaded = system->load_configuration("basic_nodes.json");
            system->init();

            GeomPayload geom_global_params;
            geom_global_params.stage = stage->get_usd_stage();
            geom_global_params.prim_path = json_path;

            system->register_global_params<GeomPayload>(geom_global_params);

            UsdBasedNodeWidgetSettings desc;

            desc.json_path = json_path;
            desc.system = system;
            desc.stage = stage.get();

            std::unique_ptr<IWidget> node_widget =
                std::move(create_node_imgui_widget(desc));

            window->register_widget(std::move(node_widget));
        }
    });

    window->run();

    unregister_cpp_type();

    window.reset();
    stage.reset();
}