#include <gtest/gtest.h>

#include <memory>

#include "GCore/GOP.h"
#include "GCore/geom_payload.hpp"
#include "GUI/window.h"
#include "Logger/Logger.h"
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

    constexpr bool use_polyscope = true;
    // Polyscope need to be initialized before window, or it cannot load opengl
    // backend correctly.
    std::unique_ptr<PolyscopeRenderer> polyscope_render;
    std::unique_ptr<PolyscopeInfoViewer> polyscope_info_viewer;
    if (use_polyscope) {
        polyscope_render = std::make_unique<PolyscopeRenderer>();
        polyscope_info_viewer = std::make_unique<PolyscopeInfoViewer>();
    }

    auto window = std::make_unique<Window>();

    auto stage = create_global_stage();
    init(stage.get());
    // Add a sphere

    auto usd_file_viewer = std::make_unique<UsdFileViewer>(stage.get());

    window->register_widget(std::move(usd_file_viewer));

    // TODO: 1. currently it cannot peacefully shutdown when there is a mesh in
    // the usd stage. need to fix it or just remove the dependency on usd stage
    // completely.
    // 2. currently the gl functions are not loaded correctly, (try remove the
    // render pointer creation), fix it probably by give a hint to which opengl
    // version you want.
    // 3. it keeps reporting "GLFW emitted error: Cannot set swap interval
    // without a current OpenGL or OpenGL ES context GLFW emitted error: Cannot
    // make current with a window that has no OpenGL or OpenGL ES context"
    if (use_polyscope) {
        window->register_widget(std::move(polyscope_render));
        window->register_widget(std::move(polyscope_info_viewer));
    }
    else {
        auto render = std::make_unique<UsdviewEngine>(stage->get_usd_stage());
        render->SetCallBack([](Window* window, IWidget* render_widget) {
            auto node_system = static_cast<const std::shared_ptr<NodeSystem>*>(
                dynamic_cast<UsdviewEngine*>(render_widget)
                    ->emit_create_renderer_ui_control());
            if (node_system) {
                FileBasedNodeWidgetSettings desc;
                desc.system = *node_system;
                desc.json_path = "render_nodes_save.json";

                std::unique_ptr<IWidget> node_widget =
                    std::move(create_node_imgui_widget(desc));

                window->register_widget(std::move(node_widget));
            }
        });
        window->register_widget(std::move(render));
    }

    window->register_function_perframe([&stage](Window* window) {
        pxr::SdfPath json_path;
        if (stage->consume_editor_creation(json_path)) {
            auto system = create_dynamic_loading_system();

            system->register_cpp_types<int>();
            auto loaded = system->load_configuration("geometry_nodes.json");
            loaded = system->load_configuration("basic_nodes.json");
            loaded = system->load_configuration("polyscope_nodes.json");
            system->init();
            system->set_node_tree_executor(create_node_tree_executor({}));

            GeomPayload geom_global_params;
            geom_global_params.stage = stage->get_usd_stage();
            geom_global_params.prim_path = json_path;

            system->set_global_params(geom_global_params);

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