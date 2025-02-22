if(USTC_CG_WITH_OPENUSD)

USTC_CG_ADD_LIB(
	stage
	SHARED
	PRIVATE_LIBS 
		Logger
		usd
		usdImaging
		nodes_system
	COMPILE_DEFS
		NOMINMAX=1
		USTC_CG_BUILD_MODULE=1
)

endif()