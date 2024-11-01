#include "widgets/usdview/usdview_widget.hpp"

#include <gtest/gtest.h>

#include "GUI/window.h"
#include "RHI/rhi.hpp"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "widgets/usdtree/usd_fileviewer.h"
using namespace USTC_CG;
TEST(USDWIDGET, create_widget)
{
    auto root_stage = pxr::UsdStage::CreateInMemory();
    // Add a sphere
    auto sphere =
        pxr::UsdGeomSphere::Define(root_stage, pxr::SdfPath("/sphere"));

    auto widget = std::make_unique<UsdFileViewer>(root_stage);
    auto render = std::make_unique<UsdviewEngine>(root_stage);

    auto window = std::make_unique<Window>();

    window->register_widget(std::move(widget));
    window->register_widget(std::move(render));

    window->run();

    window.reset();
}