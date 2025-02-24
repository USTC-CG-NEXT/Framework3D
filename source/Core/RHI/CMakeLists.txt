if(SLANG_FOUND)
	USTC_CG_ADD_LIB(
		RHI
		SHARED
		WITH_CUDA
		PUBLIC_LIBS nvrhi SLANG::slang
		PRIVATE_LIBS Vulkan::Headers Logger glfw glm::glm nodes_core
		COMPILE_DEFS 
			NOMINMAX 
			VULKAN_HPP_STORAGE_SHARED 
			VULKAN_HPP_STORAGE_SHARED_IMPORT
		PYTHON_WRAP_DIR python
	)

	if(TARGET hgi)
		target_link_libraries(RHI PUBLIC hgi)
		target_compile_definitions(RHI PUBLIC RHI_WITH_OPENUSD_HGI=1)
	endif()

	if(WIN32)
		target_compile_definitions(RHI PUBLIC USTC_CG_WITH_DX12=1)
		target_link_libraries(RHI PUBLIC d3d12 dxgi)
	endif()

	if(TARGET garch)
		target_link_libraries(RHI PUBLIC garch)
	endif()

	if(USTC_CG_WITH_CUDA)
		include(nvrtc.cmake)
	endif()

endif()
