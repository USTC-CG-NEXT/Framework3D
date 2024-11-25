USTC_CG_ADD_LIB(
	diff_optics
	PUBLIC_LIBS GUI gf RHI SLANG::slang
	PYTHON_WRAP_DIR python
)

target_compile_options(diff_optics PRIVATE -fPIC)