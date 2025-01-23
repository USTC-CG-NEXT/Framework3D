
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


target_compile_definitions(GUI PRIVATE IGFD_API=__declspec\(dllexport\))
target_compile_definitions(GUI INTERFACE IGFD_API=__declspec\(dllimport\))