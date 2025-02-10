#include <gtest/gtest.h>

#include <memory>

#include "GCore/GOP.h"
#include "GCore/geom_payload.hpp"
#include "GUI/window.h"
#include "Logger/Logger.h"
#include "imgui.h"
#include "nodes/system/node_system.hpp"
#include "nodes/ui/imgui.hpp"
#include "polyscope_widget/polyscope_info_viewer.h"
#include "polyscope_widget/polyscope_renderer.h"
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

    // Polyscope need to be initialized before window, or it cannot load opengl
    // backend correctly.
    auto polyscope_render = std::make_unique<PolyscopeRenderer>();
    polyscope_render->Set2dMode();
    // auto polyscope_info_viewer = std::make_unique<PolyscopeInfoViewer>();

    auto window = std::make_unique<Window>();

    auto stage = create_global_stage();
    init(stage.get());
    // Add a path for 2d processing

    auto usd_file_viewer = std::make_unique<UsdFileViewer>(stage.get());

    window->register_widget(std::move(usd_file_viewer));

    window->register_widget(std::move(polyscope_render));
    // window->register_widget(std::move(polyscope_info_viewer));

    window->register_function_perframe([&stage](Window* window) {
        pxr::SdfPath json_path;
        if (stage->consume_editor_creation(json_path)) {
            auto system = create_dynamic_loading_system();

            system->register_cpp_types<int>();
            auto loaded = system->load_configuration("geometry_nodes.json");
            loaded = system->load_configuration("basic_nodes.json");
            loaded = system->load_configuration("polyscope_nodes.json");
            loaded = system->load_configuration("image_warping_nodes.json");
            loaded = system->load_configuration("convert_nodes.json");
            system->init();
            system->set_node_tree_executor(create_node_tree_executor({}));

            GeomPayload global_params;
            global_params.stage = stage->get_usd_stage();
            global_params.prim_path = json_path;

            system->set_global_params(global_params);

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

    // stage.reset();
    window.reset();
    stage.reset();

    return 0;
}