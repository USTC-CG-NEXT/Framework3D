#include "RHI/internal/cuda_extension.hpp"

#include <gtest/gtest.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

using namespace USTC_CG::cuda;

TEST(cuda_extension, cuda_init)
{
    auto ret = cuda_init();
    EXPECT_EQ(ret, 0);
}

TEST(cuda_extension, optix_init)
{
    auto ret = optix_init();
    EXPECT_EQ(ret, 0);
}

TEST(cuda_extension, cuda_shutdown)
{
    auto ret = cuda_shutdown();
    EXPECT_EQ(ret, 0);
}

TEST(create_buffer, cuda_buffer)
{
    thrust::device_vector<int> d_vec(10);
    d_vec[0] = 1;
    d_vec[1] = 2;

    thrust::host_vector<int> h_vec = d_vec;
    for (int i = 0; i < h_vec.size(); i++) {
        std::cout << h_vec[i] << " ";
    }
    std::cout << std::endl;
}