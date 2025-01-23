USTC_CG_ADD_LIB(
	render_nodes_facilities
	SHARED
	PRIVATE_LIBS usd Logger RHI hdx nodes_system
	COMPILE_DEFS
		NOMINMAX 
)
target_compile_options(render_nodes_facilities PRIVATE -fPIC)