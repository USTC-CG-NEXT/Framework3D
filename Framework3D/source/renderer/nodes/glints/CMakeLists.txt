
USTC_CG_ADD_LIB(
	glints
	WITH_CUDA
	PUBLIC_LIBS RHI
	COMPILE_DEFS
		NOMINMAX 
)

target_compile_options(render_nodes_facilities PRIVATE -fPIC)