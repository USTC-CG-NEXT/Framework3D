option (ENABLE_EMBREE_BASED_RENDERER "Enable Embree based renderer" ON)

if (ENABLE_EMBREE_BASED_RENDERER)

	set(TBB_ROOT ${PROJECT_SOURCE_DIR}/SDK/OpenUSD/${SDK_FOLDER})
	add_subdirectory(embree4)
	set_target_properties(embree PROPERTIES ${OUTPUT_DIR})
#
	USTC_CG_ADD_LIB(
		hd_USTC_CG_Embree
		SHARED
		PUBLIC_LIBS Logger
		PRIVATE_LIBS usd RHI hdx nodes_system usdMtlx  usdImaging embree
		COMPILE_DEFS
			NOMINMAX
		USD_RESOURCE_DIRS resources
		SKIP_DIRS embree4
		RESOURCE_COPY_TARGET ${OUT_BINARY_DIR}/usd/hd_USTC_CG_Embree/resources
	)


endif()