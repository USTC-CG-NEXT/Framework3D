USTC_CG_ADD_LIB(
	hd_USTC_CG
	SHARED
	PRIVATE_LIBS usd Logger RHI hdx nodes_system usdMtlx
	COMPILE_DEFS
		NOMINMAX USTC_CG_DIRECT_VK_DISPLAY
	USD_RESOURCE_DIRS resources nodes/shaders
	SKIP_DIRS nodes
	RESOURCE_COPY_TARGET ${OUT_BINARY_DIR}/usd/hd_USTC_CG/resources
)

target_compile_definitions(hd_USTC_CG PRIVATE HOST_CODE=1)
target_include_directories(hd_USTC_CG PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nodes/shaders/shaders)

add_subdirectory(nodes/renderer)

add_nodes(
	TARGET_NAME render_nodes 
	SRC_DIRS nodes
	DEP_LIBS usd RHI hd_USTC_CG render_nodes_facilities
	COMPILE_DEFS NOMINMAX 
)


target_link_libraries(physical_lens_raygen PUBLIC diff_optics)