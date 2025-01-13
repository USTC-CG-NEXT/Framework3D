#pragma once
#include <glintify/api.h>

#include <vector>

#include "RHI/internal/cuda_extension.hpp"
#include "glm/glm.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class GLINTIFY_API StrokeSystem {
   public:
    std::vector<glm::vec2> get_all_endpoints();

    void set_camera_position(const glm::vec3& position)
    {
        world_camera_position = position;
    }

    void set_light_position(const glm::vec3& position)
    {
        world_light_position = position;
    }

    void calc_scratches();

   private:
    glm::vec3 world_camera_position;
    glm::vec3 world_light_position;
    std::vector<cuda::CUDALinearBufferHandle> strokes;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE