if(USTC_CG_WITH_OPENUSD)
	set(PRIVATE_LIBS Logger usd usdImaging usdImagingGL hdSt glf garch hgi RHI)
	set(COMPILE_DEFS NOMINMAX=1 USTC_CG_BUILD_MODULE=1)

set(PRIVATE_LIBS Logger GUI usd usdImaging usdImagingGL hdSt glf garch hgi RHI)
set(COMPILE_DEFS NOMINMAX=1 USTC_CG_BUILD_MODULE=1)

if (TARGET hgiVulkan)
	message(STATUS "Building usdview with Vulkan support")
	list(APPEND PRIVATE_LIBS hgiVulkan)
	list(APPEND COMPILE_DEFS USDVIEW_WITH_VULKAN=1)
endif()

USTC_CG_ADD_LIB(
	usdview_widget
	SHARED
	PUBLIC_LIBS stage
	PRIVATE_LIBS ${PRIVATE_LIBS}
	COMPILE_DEFS ${COMPILE_DEFS}
)
endif()