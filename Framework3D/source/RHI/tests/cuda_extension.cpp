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
    auto widths = create_cuda_linear_buffer(std::vector{ 0.3f, 0.3f });
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

#include "../../renderer/nodes/shaders/shaders/glints/params.h"

// TEST(cuda_extension, trace_optix_traversable)

int main()
{
    optix_init();

    std::string filename = "glints/glints.cu";

    auto raygen_group = create_optix_raygen(filename, RGS_STR(line));
    auto cylinder_module =
        get_builtin_module(OPTIX_PRIMITIVE_TYPE_ROUND_LINEAR);
    auto hg_module = create_optix_module(filename);
    OptiXProgramGroupDesc hg_desc;
    hg_desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_HITGROUP)
        .set_entry_name(nullptr, AHS_STR(line), CHS_STR(line));

    auto hg = create_optix_program_group(
        hg_desc, { cylinder_module, hg_module, hg_module });

    auto miss_group = create_optix_miss(filename, MISS_STR(line));
    auto pipeline = create_optix_pipeline({ raygen_group, hg, miss_group });

    auto line_end_vertices = create_cuda_linear_buffer(
        std::vector{ -1.f, -1.f, 0.f, 1.0f, 1.0f, 0.f });
    auto widths = create_cuda_linear_buffer(std::vector{ 0.5f, 0.5f });
    auto indices = create_cuda_linear_buffer(std::vector{ 0 });
    auto handle = create_optix_traversable(
        { line_end_vertices->get_device_ptr() },
        2,
        { widths->get_device_ptr() },
        { indices->get_device_ptr() },
        1);

    line_end_vertices->assign_host_vector<float>(
        { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f });

    handle = create_optix_traversable(
        { line_end_vertices->get_device_ptr() },
        2,
        { widths->get_device_ptr() },
        { indices->get_device_ptr() },
        1);

    const int buffer_size = 1024 * 1024;

    auto workqueue_buffer = create_cuda_linear_buffer<uint2>(buffer_size);

    auto d_workqueue = create_cuda_linear_buffer<WorkQueue<uint2>>();

    d_workqueue->assign_host_value(WorkQueue{
        reinterpret_cast<uint2*>(workqueue_buffer->get_device_ptr()) });

    auto patches_buffer = create_cuda_linear_buffer<Patch>(std::vector{
        buffer_size, Patch{ { 0, 0 }, { 0.1, 0 }, { 0.1, 0.1 }, { 0, 0.1 } } });

    auto glints_params =
        create_cuda_linear_buffer<GlintsTracingParams>(GlintsTracingParams{
            handle->getOptiXTraversable(),
            reinterpret_cast<Patch*>(patches_buffer->get_device_ptr()),
            reinterpret_cast<WorkQueue<uint2>*>(
                d_workqueue->get_device_ptr()) });

    optix_trace_ray<GlintsTracingParams>(
        handle, pipeline, glints_params->get_device_ptr(), buffer_size, 1, 1);

    auto host_workqueue = d_workqueue->get_host_value<WorkQueue<uint2>>();

    std::cout << "Workqueue size: " << host_workqueue.size << std::endl;
}

#endif