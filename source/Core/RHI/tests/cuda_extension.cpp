#if USTC_CG_WITH_CUDA
#include "RHI/internal/cuda_extension.hpp"

#include <gtest/gtest.h>

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
    auto handle = create_linear_curve_optix_traversable(
        { line_end_vertices->get_device_ptr() },
        2,
        { widths->get_device_ptr() },
        { indices->get_device_ptr() },
        1);

    EXPECT_NE(handle, nullptr);

    line_end_vertices->assign_host_vector<float>(
        { 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 2.0f });

    handle = create_linear_curve_optix_traversable(
        { line_end_vertices->get_device_ptr() },
        2,
        { widths->get_device_ptr() },
        { indices->get_device_ptr() },
        1,
        true);

    EXPECT_NE(handle, nullptr);

    // mesh handle
    // vertex buffer describing a rectangle (but in 3D space), two triangles
    auto vertex_buffer = create_cuda_linear_buffer(std::vector{ 0.0f,
                                                                0.0f,
                                                                0.0f,
                                                                1.0f,
                                                                0.0f,
                                                                0.0f,
                                                                0.0f,
                                                                1.0f,
                                                                0.0f,
                                                                1.0f,
                                                                1.0f,
                                                                0.0f });
    // index buffer referencing the two triangles
    auto index_buffer =
        create_cuda_linear_buffer(std::vector{ 0, 1, 2, 1, 2, 3 });

    auto mesh_handle = create_mesh_optix_traversable(
        { vertex_buffer->get_device_ptr() },
        4,
        3 * sizeof(float),
        index_buffer->get_device_ptr(),
        2);

    EXPECT_NE(mesh_handle, nullptr);
}

TEST(cuda_extension, get_ptx_from_cu)
{
    auto m = create_optix_module("glints/glints.cu");
    EXPECT_NE(m, nullptr);
}

TEST(cuda_extension, create_optix_pipeline)
{
    optix_init();

    std::string filename = "glints/glints.cu";

    auto raygen_group = create_optix_raygen(filename, RGS_STR(line));
    EXPECT_NE(raygen_group, nullptr);

    auto cylinder_module =
        get_builtin_module(OPTIX_PRIMITIVE_TYPE_ROUND_LINEAR);
    EXPECT_NE(cylinder_module, nullptr);

    auto hg_module = create_optix_module(filename);
    EXPECT_NE(hg_module, nullptr);

    OptiXProgramGroupDesc hg_desc;
    hg_desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_HITGROUP)
        .set_entry_name(nullptr, AHS_STR(line), CHS_STR(line));

    auto hg =
        create_optix_program_group(hg_desc, { nullptr, hg_module, hg_module });

    EXPECT_NE(hg, nullptr);

    auto miss_group = create_optix_miss(filename, MISS_STR(line));
    EXPECT_NE(miss_group, nullptr);

    auto pipeline = create_optix_pipeline({ raygen_group, hg, miss_group });

    EXPECT_NE(pipeline, nullptr);
}

#endif