USTC_CG_ADD_LIB(
	diff_optics
	PUBLIC_LIBS GUI gf RHI SLANG::slang

)
target_compile_options(diff_optics PRIVATE -fPIC)