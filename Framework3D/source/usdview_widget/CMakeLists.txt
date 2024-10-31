if(USTC_CG_WITH_OPENUSD)

set(PRIVATE_LIBS Logger GUI usd usdImaging usdImagingGL hdSt glf garch hgi hgiVulkan RHI)
set(COMPILE_DEFS NOMINMAX=1 USTC_CG_BUILD_MODULE=1)

if(TARGET Vulkan::Vulkan)
	list(APPEND PRIVATE_LIBS hgiVulkan)
	list(APPEND COMPILE_DEFS USDVIEW_WITH_VULKAN=1)
endif()

USTC_CG_ADD_LIB(
	usdview_widget
	PRIVATE_LIBS ${PRIVATE_LIBS}
	COMPILE_DEFS ${COMPILE_DEFS}
)

endif()