#pragma once
#include <glintify/api.h>

#include <vector>

#include "RHI/internal/cuda_extension.hpp"
#include "glm/glm.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class GLINTIFY_API StrokeSystem {
   public:
    std::vector<std::vector<glm::vec2>> get_all_endpoints();

    std::tuple<float*, unsigned> get_all_endpoints_in_vram();

    void set_camera_move_range(const glm::vec2& range)
    {
        camera_move_range = range;
        is_dirty = true;
    }

    void set_camera_position(const glm::vec3& position)
    {
        world_camera_position = position;
        is_dirty = true;
    }

    void set_light_position(const glm::vec3& position)
    {
        world_light_position = position;
        is_dirty = true;
    }

    void set_occlusion(
        const std::vector<glm::vec3>& vertices,
        const std::vector<unsigned>& indices);

    void calc_scratches();
    void add_virtual_point(const glm::vec3& vec);

    void clear()
    {
        strokes.clear();
    }

    void fill_ranges(bool consider_occlusion = false);

    bool is_dirty = true;

   private:
    glm::vec3 virtual_point_position;
    glm::vec3 world_camera_position;
    glm::vec2 camera_move_range;
    glm::vec3 world_light_position;
    std::vector<cuda::CUDALinearBufferHandle> strokes;
    bool on_plane_board = true;
    std::vector<std::vector<glm::vec2>> endpoints_cache;
    std::vector<glm::vec3> occlusion_vertices;
    std::vector<unsigned int> occlusion_indices;

   private:
    // Occlusion test pipeline
    std::once_flag optix_init_flag;

    void prepare_occlusion_test_pipeline();

    cuda::OptiXProgramGroupHandle miss;
    cuda::OptiXModuleHandle hg_module;
    cuda::OptiXProgramGroupHandle hit_group;
    cuda::OptiXPipelineHandle pipeline;
    cuda::OptiXProgramGroupHandle raygen;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE