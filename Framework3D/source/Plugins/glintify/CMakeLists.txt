USTC_CG_ADD_LIB(
	glintify
	SHARED
	WITH_CUDA
	PUBLIC_LIBS GUI gf RHI SLANG::slang shaders_interface glm::glm GUI OpenMeshCore glintify_cuda Logger
	PYTHON_WRAP_DIR python
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(diff_optics PRIVATE -fPIC)
endif()