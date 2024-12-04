#pragma once
#include <memory>

#include "RHI/internal/cuda_extension.hpp"
namespace USTC_CG {

struct ScratchIntersectionContext {
   public:
    ScratchIntersectionContext() = default;
    std::tuple<float*, unsigned> intersect_line_with_rays(
        float* lines,
        unsigned line_count,
        float* patches,
        unsigned patch_count,
        float width);

    void reset();

    void set_max_pair_buffer_ratio(float ratio);

   private:
    void create_raygen(std::string filename);
    void create_cylinder_intersection_shader();
    void create_hitgroup_module(std::string filename);
    void create_hitgroup();
    void create_miss_group(std::string filename);
    void create_pipeline();
    void create_width_buffer(unsigned line_count, float width);
    void create_indices(unsigned line_count);

    unsigned primitive_count;
    unsigned patch_count;
    cuda::OptiXProgramGroupHandle raygen_group;
    cuda::OptiXModuleHandle cylinder_module;
    cuda::OptiXModuleHandle hg_module;
    cuda::OptiXProgramGroupHandle hg;
    cuda::OptiXProgramGroupHandle miss_group;
    cuda::OptiXPipelineHandle pipeline;
    cuda::CUDALinearBufferHandle line_end_vertices;
    cuda::CUDALinearBufferHandle widths;
    cuda::CUDALinearBufferHandle indices;
    cuda::OptiXTraversableHandle handle;
    cuda::CUDALinearBufferHandle patches_buffer;
    cuda::CUDALinearBufferHandle glints_params;
    cuda::AppendStructuredBuffer<uint2> append_buffer;
    float _width = 0;
    unsigned _line_count = 0;
    float ratio = 1.5f;
    int _buffer_size = 0;
};

}  // namespace USTC_CG