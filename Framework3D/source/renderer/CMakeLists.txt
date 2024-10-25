file(GLOB_RECURSE internal_files ${CMAKE_CURRENT_LIST_DIR}/internal/*.cpp)
set(CPP_FILES 
	ROP.cpp
	${internal_files}
	hd_USTC_CG_GL/light.cpp
	hd_USTC_CG_GL/material.cpp
	hd_USTC_CG_GL/camera.cpp
	hd_USTC_CG_GL/renderTLAS.cpp
	hd_USTC_CG_GL/geometries/mesh.cpp
)

# Anything related to imgui should be set to STATIC
# add_library(hd_USTC_CG STATIC ${CPP_FILES})
# 
# target_link_libraries(hd_USTC_CG PUBLIC OpenMeshCore)
# 
# target_include_directories(hd_USTC_CG PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_compile_options(hd_USTC_CG PRIVATE -DUSTC_CG_BUILD_MODULE=1 -DNOMINMAX)
# set_target_properties(hd_USTC_CG PROPERTIES ${OUTPUT_DIR})
# 
# target_link_libraries(hd_USTC_CG PUBLIC usdGeom nodes SLANG::slang)
# target_compile_features(hd_USTC_CG PUBLIC cxx_std_20)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/macros)
include(pxrPlugin)

add_subdirectory(hd_USTC_CG)
target_compile_options(hd_USTC_CG PRIVATE -DUSTC_CG_BUILD_MODULE=1)