#include <gtest/gtest.h>

#include "GCore/GOP.h"
#include "GCore/geom_payload.hpp"
#include "GUI/window.h"
#include "Logger/Logger.h"
// #include "diff_optics/diff_optics.hpp"
// #include "diff_optics/lens_system.hpp"
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
#ifdef _DEBUG
    log::SetMinSeverity(Severity::Debug);
#endif
    log::EnableOutputToConsole(true);
    auto window = std::make_unique<Window>();

    auto stage = create_global_stage();
    init(stage.get());

    window->register_function_before_frame(
        [&stage](Window* window) { stage->tick(window->get_elapsed_time()); });
    // Add a sphere

    auto usd_file_viewer = std::make_unique<UsdFileViewer>(stage.get());
    auto render = std::make_unique<UsdviewEngine>(stage.get());

    auto render_bare = render.get();

    render->SetCallBack([](Window* window, IWidget* render_widget) {
        auto node_system = static_cast<const std::shared_ptr<NodeSystem>*>(
            dynamic_cast<UsdviewEngine*>(render_widget)
                ->emit_create_renderer_ui_control());
        if (node_system) {
            FileBasedNodeWidgetSettings desc;
            desc.system = *node_system;
            desc.json_path = "../../Assets/render_nodes_save.json";

            std::unique_ptr<IWidget> node_widget =
                std::move(create_node_imgui_widget(desc));

            window->register_widget(std::move(node_widget));
        }
    });

    window->register_widget(std::move(render));
    window->register_widget(std::move(usd_file_viewer));

    window->register_function_after_frame([&stage](Window* window) {
        pxr::SdfPath json_path;
        if (stage->consume_editor_creation(json_path)) {
            auto system = create_dynamic_loading_system();

            auto loaded = system->load_configuration("geometry_nodes.json");
            loaded = system->load_configuration("basic_nodes.json");
            system->init();
            system->set_node_tree_executor(create_node_tree_executor({}));

            UsdBasedNodeWidgetSettings desc;

            desc.json_path = json_path;
            desc.system = system;
            desc.stage = stage.get();

            std::unique_ptr<IWidget> node_widget =
                std::move(create_node_imgui_widget(desc));

            node_widget->SetCallBack(
                [&stage, json_path, system](Window*, IWidget*) {
                    GeomPayload geom_global_params;
                    geom_global_params.stage = stage->get_usd_stage();
                    geom_global_params.prim_path = json_path;

                    geom_global_params.has_simulation = false;

                    system->set_global_params(geom_global_params);
                });

            window->register_widget(std::move(node_widget));
        }
    });

    // std::unique_ptr<LensSystem> lens_system = std::make_unique<LensSystem>();

    //// Check existence of lens.json
    // if (std::filesystem::exists("lens.json")) {
    //     lens_system->deserialize(std::filesystem::path("lens.json"));
    // }
    // else {
    //     lens_system->set_default();
    // }

    // window->register_openable_widget(
    //     createDiffOpticsGUIFactory(), { "Plugins", "Physical Lens System" });

    // render_bare->set_renderer_setting(
    //     pxr::TfToken("lens_system_ptr"),
    //     pxr::VtValue(static_cast<void*>(lens_system.get())));

    window->register_function_after_frame(
        [render_bare](Window* window) { render_bare->finish_render(); });

    window->register_function_after_frame(
        [&stage](Window* window) { stage->finish_tick(); });

    window->run();

    unregister_cpp_type();

    window.reset();
    stage.reset();
}