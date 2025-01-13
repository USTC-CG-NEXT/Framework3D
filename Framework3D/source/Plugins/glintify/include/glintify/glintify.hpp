#pragma once
#include <glintify/api.h>

#include <vector>

#include "RHI/internal/cuda_extension.hpp"
#include "glm/glm.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class StrokeSystem {
   public:
    std::vector<std::vector<glm::vec2>> get_all_endpoints();

    void set_camera_move_range(const glm::vec2& range)
    {
        camera_move_range = range;
    }

    void set_camera_position(const glm::vec3& position)
    {
        world_camera_position = position;
    }

    void set_light_position(const glm::vec3& position)
    {
        world_light_position = position;
    }

    void calc_scratches();
    void add_virtual_point(const glm::vec3& vec);

    void clear()
    {
        strokes.clear();
    }

   private:
    void fill_ranges();

    glm::vec3 virtual_point_position;
    glm::vec3 world_camera_position;
    glm::vec2 camera_move_range;
    glm::vec3 world_light_position;
    std::vector<cuda::CUDALinearBufferHandle> strokes;
    bool consider_occlusion = false;
    bool on_plane_board = true;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE