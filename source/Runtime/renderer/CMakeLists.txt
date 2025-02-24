add_library(shaders_interface INTERFACE)
target_include_directories(shaders_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nodes/shaders)
target_compile_definitions(shaders_interface INTERFACE RENDERER_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/nodes/shaders/")

USTC_CG_ADD_LIB(
	hd_USTC_CG
	SHARED
	PUBLIC_LIBS Logger
	PRIVATE_LIBS usd RHI hdx nodes_system usdMtlx shaders_interface usdImaging
	COMPILE_DEFS
		NOMINMAX USTC_CG_DIRECT_VK_DISPLAY
	USD_RESOURCE_DIRS resources nodes/shaders
	SKIP_DIRS nodes
	RESOURCE_COPY_TARGET ${OUT_BINARY_DIR}/usd/hd_USTC_CG/resources
)

target_compile_definitions(hd_USTC_CG PRIVATE HOST_CODE=1)
target_include_directories(hd_USTC_CG PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/nodes/shaders/shaders)
target_include_directories(hd_USTC_CG PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

add_nodes(
	TARGET_NAME render_nodes 
	SRC_DIRS nodes
	DEP_LIBS usd RHI hd_USTC_CG usdImaging shaders_interface
	COMPILE_DEFS NOMINMAX 
	EXTRA_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source
)


