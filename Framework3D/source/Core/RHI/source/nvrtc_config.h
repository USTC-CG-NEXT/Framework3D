#pragma once

#define OptiX_DIR ""
#define OptiX_PTX_DIR ""
#define OptiX_CUDA_DIR ""

// Include directories
#define OptiX_RELATIVE_INCLUDE_DIRS \
  "include", \
  ".",  "/include",
#define OptiX_ABSOLUTE_INCLUDE_DIRS \
  "C:/Users/Jerry/WorkSpace/USTC_CG_NEXT/Framework3D/source/Core/RHI/include/RHI/internal/optix", \
  "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6/include/cuda/std", \
  "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6/include",  ""

// Signal whether to use NVRTC or not
#define CUDA_NVRTC_ENABLED 1

// NVRTC compiler options
#define CUDA_NVRTC_OPTIONS  \
  "-std=c++17", \
  "-arch", \
  "compute_70", \
  "-lineinfo", \
  "-use_fast_math", \
  "-default-device", \
  "-rdc", \
  "true", \
  "-D__x86_64",
