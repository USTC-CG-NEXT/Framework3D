#include "widgets/usdview/usdview_widget.hpp"

#include <gtest/gtest.h>

#include "GUI/window.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "widgets/usdtree/usd_fileviewer.h"
using namespace USTC_CG;

class UsdviewEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        root_stage = pxr::UsdStage::CreateInMemory();
        view_engine = std::make_unique<UsdviewEngine>(root_stage);
    }

    void TearDown() override {
        view_engine.reset();
    }

    pxr::UsdStageRefPtr root_stage;
    std::unique_ptr<UsdviewEngine> view_engine;
};

TEST_F(UsdviewEngineTest, CreateWidget) {
    ASSERT_NE(view_engine, nullptr);
    // Add more assertions and tests as needed
}

TEST_F(UsdviewEngineTest, AddSphere) {
    auto sphere = pxr::UsdGeomSphere::Define(root_stage, pxr::SdfPath("/sphere"));
    ASSERT_TRUE(sphere);
    // Add more assertions and tests as needed
}