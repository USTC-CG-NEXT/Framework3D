USTC_CG_ADD_LIB(
	hd_USTC_CG
	SHARED
	PRIVATE_LIBS usd Logger RHI hdx nodes_system
	COMPILE_DEFS
		NOMINMAX 
	USD_RESOURCE_DIRS resources
	SKIP_DIRS nodes
	RESOURCE_COPY_TARGET ${OUT_BINARY_DIR}/usd/hd_USTC_CG/resources
)

add_subdirectory(nodes/renderer)

add_nodes(
	TARGET_NAME render_nodes 
	SRC_DIRS nodes
	DEP_LIBS usd RHI hd_USTC_CG render_nodes_facilities
	COMPILE_DEFS NOMINMAX 
)
