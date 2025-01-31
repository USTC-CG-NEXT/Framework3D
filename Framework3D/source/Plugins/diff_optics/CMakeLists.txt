USTC_CG_ADD_LIB(
	diff_optics
	PUBLIC_LIBS GUI gf RHI SLANG::slang shaders_interface usdview_widget usd
	PYTHON_WRAP_DIR python
	COMPILE_DEFS NOMINMAX
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(diff_optics PRIVATE -fPIC)
endif()