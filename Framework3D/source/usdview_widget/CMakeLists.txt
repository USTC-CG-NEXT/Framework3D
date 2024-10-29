if(USTC_CG_WITH_OPENUSD)

USTC_CG_ADD_LIB(
	usdview_widget
	SHARED
	PRIVATE_LIBS 
		Logger
		GUI
		usd
		usdImaging
	COMPILE_DEFS
		NOMINMAX=1
		USTC_CG_BUILD_MODULE=1
)

endif()