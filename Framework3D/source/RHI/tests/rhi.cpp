#include "RHI/rhi.hpp"

#include <gtest/gtest.h>

TEST(CreateRHI, create_rhi)
{
    EXPECT_TRUE(USTC_CG::RHI::init());
    EXPECT_TRUE(USTC_CG::RHI::get_device() != nullptr);
    EXPECT_TRUE(USTC_CG::RHI::shutdown());
}

TEST(CreateRHI, create_rhi_with_window)
{
    EXPECT_TRUE(USTC_CG::RHI::init(true));
    EXPECT_TRUE(USTC_CG::RHI::get_device() != nullptr);
    EXPECT_TRUE(USTC_CG::RHI::internal::get_device_manager() != nullptr);
    EXPECT_TRUE(USTC_CG::RHI::shutdown());
}

#ifndef __linux__
TEST(CreateRHI, create_rhi_with_dx12)
{
    EXPECT_TRUE(USTC_CG::RHI::init(false, true));
    EXPECT_TRUE(USTC_CG::RHI::get_device() != nullptr);
    EXPECT_TRUE(USTC_CG::RHI::internal::get_device_manager() != nullptr);
    EXPECT_TRUE(USTC_CG::RHI::shutdown());
}

TEST(CreateRHI, create_rhi_with_window_and_dx12)
{
    EXPECT_TRUE(USTC_CG::RHI::init(true, true));
    EXPECT_TRUE(USTC_CG::RHI::get_device() != nullptr);
    EXPECT_TRUE(USTC_CG::RHI::internal::get_device_manager() != nullptr);
    EXPECT_TRUE(USTC_CG::RHI::shutdown());
}
#endif