#include <gtest/gtest.h>

#include "GUI/window.h"
#include "Logger/Logger.h"
#include "RHI/rhi.hpp"
#include "nodes/system/node_system.hpp"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "stage/stage.hpp"
#include "widgets/usdtree/usd_fileviewer.h"
#include "widgets/usdview/usdview_widget.hpp"
using namespace USTC_CG;
int main()
{
    log::SetMinSeverity(Severity::Debug);
    log::EnableOutputToConsole(true);
    auto stage = create_global_stage();
    // Add a sphere

    stage->create_sphere(pxr::SdfPath("/sphere"));

    auto widget = std::make_unique<UsdFileViewer>(stage.get());
    auto render = std::make_unique<UsdviewEngine>(stage->get_usd_stage());

    auto window = std::make_unique<Window>();

    window->register_widget(std::move(widget));
    window->register_widget(std::move(render));

    auto system = create_dynamic_loading_system();

    window->check_true_execute(
        widget.get(),
        [](IWidget* widget) {
            return !dynamic_cast<UsdFileViewer*>(widget)
                        ->emit_editor_info_path()
                        .IsEmpty();
        },
        []() {
            window->register_widget()
        });

    window->run();

    window.reset();
}