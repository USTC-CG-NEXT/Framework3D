find_package(CUDAToolkit REQUIRED)

USTC_CG_ADD_LIB(
	glints
	PUBLIC_LIBS RHI
	PRIVATE_LIBS CUDA::nvrtc 
	COMPILE_DEFS
		NOMINMAX 
)

target_compile_options(render_nodes_facilities PRIVATE -fPIC)