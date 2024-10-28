#include "RHI/rhi.hpp"

#include <gtest/gtest.h>

TEST(CreateRHI, create_rhi)
{
    EXPECT_TRUE(USTC_CG::rhi::init());
    EXPECT_TRUE(USTC_CG::rhi::get_device() != nullptr);
    EXPECT_TRUE(USTC_CG::rhi::shutdown());
}

TEST(CreateRHI, create_rhi_with_window)
{
    EXPECT_TRUE(USTC_CG::rhi::init(true));
    EXPECT_TRUE(USTC_CG::rhi::get_device() != nullptr);
    EXPECT_TRUE(USTC_CG::rhi::internal::get_device_manager() != nullptr);
    EXPECT_TRUE(USTC_CG::rhi::shutdown());
}
