USTC_CG_ADD_LIB(
	nodes_system
	SHARED
	PUBLIC_LIBS nodes_core
	PRIVATE_LIBS Logger
	COMPILE_DEFS 
		USTC_CG_BUILD_MODULE=1 
)
add_subdirectory(tests)