#pragma once
#include <memory>

#include "RHI/internal/cuda_extension.hpp"
#include "shaders/glints/mesh_params.h"

namespace USTC_CG {

struct MeshIntersectionContext {
    MeshIntersectionContext() = default;
    ~MeshIntersectionContext();
    // Returned structure:
    std::tuple<float*, float*, unsigned*, unsigned> intersect_mesh_with_rays(
        float* vertices,
        unsigned vertices_count,
        unsigned vertex_buffer_stride,
        float* indices,
        unsigned index_count,
        int2 resolution,
        const std::vector<float>& world_to_clip);

   private:
    void create_raygen(const std::string& string);
    void create_hitgroup_module(const std::string& string);
    void create_hitgroup();
    void create_miss_group(const std::string& string);
    void create_pipeline();
    void ensure_pipeline();

    cuda::CUDALinearBufferHandle vertex_buffer;
    cuda::CUDALinearBufferHandle index_buffer;
    cuda::OptiXPipelineHandle pipeline;
    cuda::OptiXProgramGroupHandle raygen_group;
    cuda::OptiXModuleHandle hg_module;
    cuda::OptiXProgramGroupHandle hg;
    cuda::OptiXProgramGroupHandle miss_group;
    cuda::AppendStructuredBuffer<Patch> append_buffer;
    cuda::CUDALinearBufferHandle target_buffer;
    cuda::OptiXTraversableHandle handle;
    cuda::CUDALinearBufferHandle corners_buffer;
};

}  // namespace USTC_CG