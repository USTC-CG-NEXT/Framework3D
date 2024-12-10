
USTC_CG_ADD_LIB(
	glints
	WITH_CUDA
	PUBLIC_LIBS RHI
	COMPILE_DEFS
		NOMINMAX 
)

if (NOT WIN32)
	target_compile_options(glints PRIVATE -fPIC)
endif()
