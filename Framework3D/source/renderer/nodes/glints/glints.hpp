#pragma once
#include <memory>

#include "RHI/internal/cuda_extension.hpp"
namespace USTC_CG {

#define required_information float *x1, float *y1, float *x2, float *y2

// OptiXTLASHandle build_line_tlas_handle(required_information);
// OptiXTLASHandle rebuild_line_tlas_handle(
//     OptiXTLASHandle handle,
//     required_information);

}  // namespace USTC_CG