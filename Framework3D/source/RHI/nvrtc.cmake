set(CUDA_NVRTC_ENABLED 1)
set(OptiX_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/include/RHI/internal/optix)
message("Optix include at ${OptiX_INCLUDE}")

set(OptiX_ABSOLUTE_INCLUDE_DIRS "\\
  \"${OptiX_INCLUDE}\", \\
  \"${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}/cuda/std\", \\
  \"${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}\", ")

  # NVRTC include paths relative to the sample path
set(OptiX_RELATIVE_INCLUDE_DIRS "\\
  \"include\", \\
  \".\", ")

set(CUDA_NVRTC_FLAGS -std=c++17 -arch compute_70 -lineinfo -use_fast_math -default-device -rdc true -D__x86_64 CACHE STRING "Semi-colon delimit multiple arguments." FORCE)
mark_as_advanced(CUDA_NVRTC_FLAGS)

# Build a null-terminated option list for NVRTC
set(CUDA_NVRTC_OPTIONS)
foreach(flag ${CUDA_NVRTC_FLAGS})
  set(CUDA_NVRTC_OPTIONS "${CUDA_NVRTC_OPTIONS} \\\n  \"${flag}\",")
endforeach()
set(CUDA_NVRTC_OPTIONS "${CUDA_NVRTC_OPTIONS}")

message( "list dir: ${CMAKE_CURRENT_LIST_DIR}")
message( "source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

configure_file(${CMAKE_CURRENT_LIST_DIR}/source/nvrtc_config.h.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/nvrtc_config.h @ONLY)

