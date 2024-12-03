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

TEST(cuda_extension, create_linear_buffer)
{
    auto desc = CUDALinearBufferDesc(10, 4);
    auto handle = create_cuda_linear_buffer(desc);
    CUdeviceptr device_ptr = handle->get_device_ptr();
    EXPECT_NE(device_ptr, 0);

    auto handle2 = create_cuda_linear_buffer(std::vector{ 1, 2, 3, 4 });

    auto device_ptr2 = handle2->get_device_ptr();
    EXPECT_NE(device_ptr2, 0);

    auto host_vec = handle2->get_host_vector<int>();

    EXPECT_EQ(host_vec.size(), 4);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(host_vec[i], i + 1);
    }
}

TEST(cuda_extension, create_optix_traversable)
{
    optix_init();

    auto line_end_vertices = create_cuda_linear_buffer(
        std::vector{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });
    auto widths = create_cuda_linear_buffer(std::vector{ 0.1f, 0.1f });
    auto indices = create_cuda_linear_buffer(std::vector{ 0 });
    auto handle = create_optix_traversable(
        { line_end_vertices->get_device_ptr() },
        2,
        { widths->get_device_ptr() },
        { indices->get_device_ptr() },
        1);

    EXPECT_NE(handle, nullptr);

    line_end_vertices->assign_host_vector<float>(
        { 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f });

    handle = create_optix_traversable(
        { line_end_vertices->get_device_ptr() },
        2,
        { widths->get_device_ptr() },
        { indices->get_device_ptr() },
        1,
        true);

    EXPECT_NE(handle, nullptr);
}

TEST(cuda_extension, get_ptx_from_cu)
{
    std::string ptx = get_ptx_string_from_cu("glints/glints.cu");
    std::cout << ptx;
}

// TEST(cuda_extension, create_optix_pipeline)
//{
//     optix_init();
//
//     std::string filename;
//     std::string entry_name;
//     auto raygen_group = create_optix_raygen(filename, entry_name);
//
//     EXPECT_NE(raygen_group, nullptr);
//
//     auto cylinder_module = get_builtin_module();
//     EXPECT_NE(cylinder_module, nullptr);
//
//     auto hit_group1 = create_optix_hitgroup();
//     EXPECT_NE(hit_group1, nullptr);
//
//     auto miss_group = create_optix_miss();
//     EXPECT_NE(miss_group, nullptr);
//
//     auto pipeline = create_optix_pipeline(
//         { {}, {} }, { raygen_group, hit_group1, miss_group });
// }

TEST(cuda_extension, trace_optix_traversable)
{
    // optix_init();
    // auto line_end_vertices = create_cuda_linear_buffer(
    //     std::vector{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f });
    // auto widths = create_cuda_linear_buffer(std::vector{ 0.1f, 0.1f });
    // auto indices = create_cuda_linear_buffer(std::vector{ 0 });
    // auto traversable = create_optix_traversable(
    //     { line_end_vertices->get_device_ptr() },
    //     2,
    //     { widths->get_device_ptr() },
    //     { indices->get_device_ptr() },
    //     1);
    // auto raygen_group = create_optix_raygen("raygen.ptx", "raygen");
    // auto hit_group1 = create_optix_hitgroup("hitgroup.ptx", "closest_hit");
    // auto miss_group = create_optix_miss("miss.ptx", "miss");
    // auto pipeline = create_optix_pipeline(
    //     { {}, {} }, { raygen_group, hit_group1, miss_group });
    // auto handle = trace_optix_traversable(
    //     pipeline,
    //     traversable,
    //     { 0.0f, 0.0f, 0.0f },
    //     { 1.0f, 1.0f, 1.0f },
    //     0.1f);
}