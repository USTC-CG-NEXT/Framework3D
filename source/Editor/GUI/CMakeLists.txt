
USTC_CG_ADD_LIB(
	GUI
	SHARED
	PUBLIC_LIBS imgui
	PRIVATE_LIBS Logger RHI stb_image
	COMPILE_DEFS 
		NOMINMAX 
		VULKAN_HPP_STORAGE_SHARED 
		VULKAN_HPP_STORAGE_SHARED_IMPORT
)


#target_compile_definitions(GUI PRIVATE IGFD_API=__declspec\(dllexport\))
#target_compile_definitions(GUI INTERFACE IGFD_API=__declspec\(dllimport\))

if(MSVC)
       target_compile_definitions(GUI PRIVATE IGFD_API=__declspec\(dllexport\))
       target_compile_definitions(GUI INTERFACE IGFD_API=__declspec\(dllimport\))
else()
       target_compile_definitions(imgui PRIVATE IGFD_API=__attribute__\(\(visibility\(\"default\"\)\)\))
        target_compile_options(imgui PRIVATE -fvisibility=hidden)
        target_compile_definitions(imgui INTERFACE IGFD_API=__attribute__\(\(visibility\(\"default\"\)\)\))
        target_compile_options(imgui INTERFACE -fvisibility=hidden)
endif()
