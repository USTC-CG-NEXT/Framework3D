cmake_minimum_required(VERSION 3.19)

#
# Project details
#
project(
  "USTC_CG_2024"
  VERSION 1.0
  LANGUAGES C CXX
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OUT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/${CMAKE_BUILD_TYPE})
message("Build binaries to ${OUT_BINARY_DIR}")
set(OUTPUT_DIR
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUT_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUT_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUT_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${OUT_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUT_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUT_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUT_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${OUT_BINARY_DIR}"
)

# For testing
enable_testing()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/googletest)
set_target_properties(gtest PROPERTIES ${OUTPUT_DIR})
set_target_properties(gtest_main PROPERTIES ${OUTPUT_DIR})

include(cmake/AddLibrary.cmake)
include(cmake/AddNodes.cmake)

option(USTC_CG_WITH_CUDA OFF)
option(USTC_CG_WITH_TORCH OFF)

if(USTC_CG_WITH_CUDA)
  # Set the cuda compiler to be nvcc 12.6
  set(CMAKE_CUDA_ARCHITECTURES 86)
  enable_language(CUDA)
  find_package(CUDAToolkit REQUIRED)
  find_package(CCCL REQUIRED)
  add_compile_definitions(USTC_CG_WITH_CUDA=1)
  set(CMAKE_CUDA_STANDARD 20)
else()
  add_compile_definitions(USTC_CG_WITH_CUDA=0)
endif()

add_compile_definitions(BOOST_PYTHON_NO_LIB=1)

message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")
set(BUILD_SHARED_LIBS ON)

# On mac, the cpp standard should >= 20
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_CUDA_STANDARD 20)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  set(SDK_FOLDER ${CMAKE_BUILD_TYPE})
elseif(${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
  set(SDK_FOLDER ${CMAKE_BUILD_TYPE})
else()
  set(SDK_FOLDER "Release")
endif()

message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}. Selecting SDK from ${SDK_FOLDER}")

if(USTC_CG_WITH_TORCH)
  set(Torch_DIR SDK/${SDK_FOLDER}/libtorch/share/cmake/Torch)
  find_package(Torch REQUIRED)
  add_compile_definitions(USTC_CG_WITH_TORCH=1)
else()
  add_compile_definitions(USTC_CG_WITH_TORCH=0)
endif()

# The following packages optional
find_package(Python COMPONENTS Development Interpreter REQUIRED)

find_package(Vulkan)

if(Vulkan_FOUND)
  message("Found Vulkan version ${Vulkan_VERSION}")
else()
  message("Vulkan not found")
endif()

set(pxr_DIR SDK/OpenUSD/${SDK_FOLDER})
find_package(pxr)

if(pxr_FOUND)
  message("Found OpenUSD version ${pxr_VERSION}")
  find_package(OpenGL REQUIRED)
  set(USTC_CG_WITH_OPENUSD ON)

  if(${CMAKE_BUILD_TYPE} MATCHES "Debug")

    if(Python3_LIBRARY MATCHES "_d.lib$")
      set_target_properties(usd PROPERTIES INTERFACE_COMPILE_DEFINITIONS "TBB_USE_DEBUG=1; BOOST_DEBUG_PYTHON")
    else()
      set_target_properties(usd PROPERTIES INTERFACE_COMPILE_DEFINITIONS "TBB_USE_DEBUG=1")
    endif()
  endif()
else()
  set(USTC_CG_WITH_OPENUSD OFF)
endif()

# Include Slang
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(WIN32)
  find_package(NVAPI REQUIRED)
endif()

set(SLANG_ROOT_DIR ${PROJECT_SOURCE_DIR}/SDK/slang)
find_package(Slang REQUIRED COMPONENTS slang slang-rt gfx)

add_subdirectory(external)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/Core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/Editor)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/Runtime)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/Plugins)

add_subdirectory(tests)