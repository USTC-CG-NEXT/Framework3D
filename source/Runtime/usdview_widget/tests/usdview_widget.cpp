#include "widgets/usdview/usdview_widget.hpp"

#include <gtest/gtest.h>

#include "GUI/window.h"
#include "Logger/Logger.h"
#include "RHI/rhi.hpp"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "stage/stage.hpp"
#include "widgets/usdtree/usd_fileviewer.h"
using namespace USTC_CG;
int main()
{
    log::SetMinSeverity(Severity::Debug);
    log::EnableOutputToConsole(true);

    auto stage = create_global_stage();
    // Add a sphere
    stage->create_sphere(pxr::SdfPath("/sphere"));

    auto widget = std::make_unique<UsdFileViewer>(stage.get());
    auto render = std::make_unique<UsdviewEngine>(stage.get());

    auto window = std::make_unique<Window>();

    window->register_widget(std::move(widget));
    window->register_widget(std::move(render));

    window->run();

    window.reset();
}