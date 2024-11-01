set(SLANG_ROOT_DIR ${PROJECT_SOURCE_DIR}/SDK/slang)
find_package(Slang REQUIRED COMPONENTS slang slang-rt gfx)

if(SLANG_FOUND)
	USTC_CG_ADD_LIB(
		RHI
		SHARED
		PUBLIC_LIBS nvrhi SLANG::slang
		PRIVATE_LIBS Vulkan::Headers Logger DXC::dxc glfw glm::glm
		COMPILE_DEFS 
			USTC_CG_BUILD_MODULE=1 
			NOMINMAX 
			VULKAN_HPP_STORAGE_SHARED 
			VULKAN_HPP_STORAGE_SHARED_IMPORT
	)

	if(TARGET hgi)
		target_link_libraries(RHI PUBLIC hgi)
		target_compile_definitions(RHI PUBLIC RHI_WITH_OPENUSD_HGI=1)
	endif()

	if(TARGET garch)
		target_link_libraries(RHI PUBLIC garch)
	endif()
endif()
