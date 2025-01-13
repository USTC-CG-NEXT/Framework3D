#include <glintify/glintify.hpp>

#include "stroke.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace stroke {

// Another question would be how to consider the luminance? the shading?
// By controlling the density of the scratches.
// But how does that mean exactly?

void Stroke::calc_scratch(int scratch_index)
{
    atomicAdd(&scratch_count, 1);
}

void calc_scratches(
    cuda::CUDALinearBufferHandle strokes,
    glm::vec3 camera_position,
    glm::vec3 light_position)
{
    auto stroke_count = strokes->getDesc().element_count;

    unsigned calculation_load = stroke_count * MAX_SCRATCH_COUNT;

    Stroke** d_strokes_ptr =
        reinterpret_cast<Stroke**>(strokes->get_device_ptr());

    GPUParallelFor(
        "calc_scratches", calculation_load, GPU_LAMBDA_Ex(int index) {
            auto related_stroke = index / MAX_SCRATCH_COUNT;
            auto scratch_index = index % MAX_SCRATCH_COUNT;
            auto stroke = d_strokes_ptr[related_stroke];

            stroke->calc_scratch(scratch_index);
        });
}

}  // namespace stroke

USTC_CG_NAMESPACE_CLOSE_SCOPE