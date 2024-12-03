#pragma once

#define Optika_DIR "C:/Users/Jerry/WorkSpace/Optika"
#define Optika_PTX_DIR "C:/Users/Jerry/WorkSpace/Optika/lib/ptx"
#define Optika_CUDA_DIR "C:/Users/Jerry/WorkSpace/Optika/src/OptiX"

// Include directories
#define Optika_RELATIVE_INCLUDE_DIRS \
  "include", \
  ".", 
#define Optika_ABSOLUTE_INCLUDE_DIRS \
  "C:/ProgramData/NVIDIA Corporation/OptiX SDK 7.2.0/include", \
  "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/include", 

// Signal whether to use NVRTC or not
#define CUDA_NVRTC_ENABLED 1

// NVRTC compiler options
#define CUDA_NVRTC_OPTIONS  \
  "-std=c++17", \
  "-arch", \
  "compute_61", \
  "-lineinfo", \
  "-use_fast_math", \
  "-default-device", \
  "-rdc", \
  "true", \
  "-D__x86_64",
