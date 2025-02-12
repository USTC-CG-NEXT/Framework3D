set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
add_subdirectory(eigen)

if(WIN32)
    target_compile_options(eigen INTERFACE /wd5054)
endif()

# glfw
add_subdirectory(glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set_target_properties(glfw update_mappings PROPERTIES FOLDER "external/GLFW3")
set_target_properties(glfw update_mappings PROPERTIES ${OUTPUT_DIR})

# nvrhi (with vulkan headers)
if(WIN32)
    set(NVRHI_WITH_NVAPI ON)
endif()

set(NVRHI_BUILD_SHARED ON CACHE INTERNAL "Override default shared build of nvrhi")
add_subdirectory(nvrhi)
set_target_properties(nvrhi PROPERTIES ${OUTPUT_DIR})

if(WIN32)
    target_link_libraries(nvrhi PUBLIC NVAPI::NVAPI)
endif()

# imgui
set(imgui_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui" CACHE STRING "")
set(imgui_file_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog" CACHE STRING "")
file(GLOB source
    "${imgui_SOURCE_DIR}/*.h"
    "${imgui_SOURCE_DIR}/*.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.*"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.*"
    "${imgui_file_SOURCE_DIR}/*.h"
    "${imgui_file_SOURCE_DIR}/*.cpp"
)
add_library(imgui SHARED ${source})
target_include_directories(imgui
    PUBLIC "${imgui_SOURCE_DIR}"
    PUBLIC "${imgui_SOURCE_DIR}/backends"
    PUBLIC "${imgui_file_SOURCE_DIR}"
)
target_include_directories(imgui PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/user_imgui")
target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="imgui_user_config.h")
target_compile_definitions(imgui PRIVATE IMGUI_API=__declspec\(dllexport\))
target_link_libraries(imgui PUBLIC glfw Vulkan::Headers)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES=1)
set_target_properties(imgui PROPERTIES FOLDER "external" ${OUTPUT_DIR})

add_subdirectory(OpenMesh)
set_target_properties(OpenMeshCore PROPERTIES FOLDER "external")
set_target_properties(OpenMeshTools PROPERTIES FOLDER "external")

target_compile_definitions(OpenMeshCore PUBLIC -D_USE_MATH_DEFINES=1)

add_subdirectory(entt)
add_subdirectory(glm)

set_target_properties(glm PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_target_properties(glm PROPERTIES ${OUTPUT_DIR})
set(GLM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glm" CACHE STRING "")
add_subdirectory(nanobind)
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)
set_target_properties(glm PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(stb_image)

# polyscope
add_subdirectory(polyscope)
set_target_properties(polyscope PROPERTIES ${OUTPUT_DIR})
set_target_properties(glad PROPERTIES ${OUTPUT_DIR})


# libigl
set(LIBIGL_RESTRICTED_TRIANGLE ON CACHE BOOL "" FORCE)
add_subdirectory(libigl)
set_target_properties(igl_core PROPERTIES ${OUTPUT_DIR})
set_target_properties(polyscope PROPERTIES ${OUTPUT_DIR})

# autodiff
set(AUTODIFF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(AUTODIFF_BUILD_PYTHON OFF CACHE BOOL "" FORCE)
set(AUTODIFF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(AUTODIFF_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(autodiff)
set_target_properties(autodiff PROPERTIES ${OUTPUT_DIR})
