

#include "widgets/usdtree/usd_fileviewer.h"

#include <gtest/gtest.h>

#include "GUI/window.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "widgets/usdview/usdview_widget.hpp"
using namespace USTC_CG;

TEST(USDWIDGET, create_widget)
{
    auto root_stage = pxr::UsdStage::CreateInMemory();
    auto sphere =
        pxr::UsdGeomSphere::Define(root_stage, pxr::SdfPath("/sphere"));

    auto widget = std::make_unique<UsdFileViewer>(root_stage);
    auto window = std::make_unique<Window>();
    window->register_widget(std::move(widget));
    window->run();
    window.reset();
}