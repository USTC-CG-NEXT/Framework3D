#include <gtest/gtest.h>

#include <stage/stage.hpp>

#include "pxr/usd/usd/prim.h"

using namespace USTC_CG;

TEST(Stage, CreateStage)
{
    Stage stage;
    auto prim = stage.add_prim(pxr::SdfPath("/root"));
    ASSERT_TRUE(prim);

    auto content = stage.stage_content();
    ASSERT_FALSE(content.empty());
}