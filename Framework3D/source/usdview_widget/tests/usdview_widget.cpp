
#include <gtest/gtest.h>

#include "GUI/window.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "widgets/usdview/usdview.hpp"
using namespace USTC_CG;

TEST(USDWIDGET, create_widget)
{
    auto root_stage = pxr::UsdStage::CreateInMemory();
    // Add a sphere
    auto sphere =
        pxr::UsdGeomSphere::Define(root_stage, pxr::SdfPath("/sphere"));

    auto widget = std::make_unique<UsdviewEngine>(root_stage);

    auto window = std::make_unique<Window>();

    window->register_widget(std::move(widget));

    window->run();

    window.reset();
}