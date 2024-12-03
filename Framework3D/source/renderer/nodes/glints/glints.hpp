#pragma once
#include <memory>

#include "RHI/internal/nvrhi_cuda_extension.hpp"
namespace USTC_CG {

struct OptiXTLAS;
using OptiXTLASHandle = std::shared_ptr<OptiXTLAS>;

void optix_init();

#define required_information float *x1, float *y1, float *x2, float *y2

OptiXTLASHandle build_line_tlas_handle(required_information);
OptiXTLASHandle rebuild_line_tlas_handle(
    OptiXTLASHandle handle,
    required_information);

void optix_shutdown();

}  // namespace USTC_CG