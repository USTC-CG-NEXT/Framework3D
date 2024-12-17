#include "../nodes/glints/glints.hpp"
#if USTC_CG_WITH_CUDA

#include <gtest/gtest.h>

#include "RHI/internal/cuda_extension.hpp"
#include "shaders/glints/params.h"
using namespace USTC_CG::cuda;
using namespace USTC_CG;

TEST(cuda_extension, trace_optix_mesh_traversable)
{
    MeshIntersectionContext context;

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
    auto index_buffer =
        create_cuda_linear_buffer(std::vector{ 0, 1, 2, 1, 2, 3 });

    auto ray_count = 1024 * 1024;

    std::vector<Ray> rays(ray_count);

    for (int i = 0; i < 1024; ++i) {
        for (int j = 0; j < 1024; ++j) {
            float step = 2.f / 1024;
            float x = -1 + i * step;
            float y = -1 + j * step;
            rays[i * 1024 + j] = { { x, y, 1.0f }, { 0, 0, -1 }, 0, 1000 };
        }
    }

    std::vector<float> worldToClip = { 2.0f, 0.0f, 0.0f,  0.0f, 0.0f,  2.0f,
                                       0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
                                       0.0f, 0.0f, -1.0f, 0.0f };

    auto ray_buffer = create_cuda_linear_buffer<Ray>(rays);

    context.intersect_mesh_with_rays(
        reinterpret_cast<float*>(vertex_buffer->get_device_ptr()),
        4,
        3 * sizeof(float),
        reinterpret_cast<float*>(index_buffer->get_device_ptr()),
        2,
        { 1024, 1024 },
        worldToClip);
}
//
// TEST(cuda_extension, trace_optix_lines_traversable)
//{
//    optix_init();
//
//    std::string filename =
//        std::string(RENDERER_SHADER_DIR) + "shaders/glints/glints.cu";
//
//    auto raygen_group = create_optix_raygen(filename, RGS_STR(line));
//    auto cylinder_module =
//        get_builtin_module(OPTIX_PRIMITIVE_TYPE_ROUND_LINEAR);
//    auto hg_module = create_optix_module(filename);
//    OptiXProgramGroupDesc hg_desc;
//    hg_desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_HITGROUP)
//        .set_entry_name(nullptr, AHS_STR(line), CHS_STR(line));
//
//    auto hg = create_optix_program_group(
//        hg_desc, { cylinder_module, hg_module, hg_module });
//
//    auto miss_group = create_optix_miss(filename, MISS_STR(line));
//    auto pipeline = create_optix_pipeline({ raygen_group, hg, miss_group });
//
//    auto line_end_vertices = create_cuda_linear_buffer(
//        std::vector{ -1.f, -1.f, 0.f, 1.0f, 1.0f, 0.f });
//    auto widths = create_cuda_linear_buffer(std::vector{ 0.5f, 0.5f });
//    auto indices = create_cuda_linear_buffer(std::vector{ 0 });
//    auto handle = create_linear_curve_optix_traversable(
//        { line_end_vertices->get_device_ptr() },
//        2,
//        { widths->get_device_ptr() },
//        { indices->get_device_ptr() },
//        1);
//
//    line_end_vertices->assign_host_vector<float>(
//        { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f });
//
//    handle = create_linear_curve_optix_traversable(
//        { line_end_vertices->get_device_ptr() },
//        2,
//        { widths->get_device_ptr() },
//        { indices->get_device_ptr() },
//        1);
//
//    const int buffer_size = 1024 * 1024;
//
//    AppendStructuredBuffer<uint2> append_buffer(buffer_size);
//
//    std::vector<Patch> patches;
//    patches.reserve(1024 * 1024);
//
//    for (int i = 0; i < 1024; ++i) {
//        for (int j = 0; j < 1024; ++j) {
//            float step = 2.f / 1024;
//            float x = -1 + i * step;
//            float y = -1 + j * step;
//            patches.push_back({ { x, y },
//                                { x + step, y },
//                                { x + step, y + step },
//                                { x, y + step } });
//        }
//    }
//
//    auto patches_buffer = create_cuda_linear_buffer<Patch>(patches);
//
//    auto glints_params =
//        create_cuda_linear_buffer<GlintsTracingParams>(GlintsTracingParams{
//            handle->getOptiXTraversable(),
//            reinterpret_cast<Patch*>(patches_buffer->get_device_ptr()),
//            append_buffer.get_device_queue_ptr() });
//
//    optix_trace_ray<GlintsTracingParams>(
//        handle, pipeline, glints_params->get_device_ptr(), buffer_size, 1, 1);
//
//    EXPECT_EQ(408443, append_buffer.get_size());
//
//    append_buffer.reset();
//
//    EXPECT_EQ(0, append_buffer.get_size());
//
//    optix_trace_ray<GlintsTracingParams>(
//        handle, pipeline, glints_params->get_device_ptr(), buffer_size, 1, 1);
//    EXPECT_EQ(408443, append_buffer.get_size());
//}

#endif