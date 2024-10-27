#include "RHI/rhi.hpp"

#include <gtest/gtest.h>

TEST(CreateRHI, create_rhi)
{
    EXPECT_TRUE(USTC_CG::init());
    EXPECT_TRUE(USTC_CG::get_device() != nullptr);
    EXPECT_TRUE(USTC_CG::shutdown());
}

