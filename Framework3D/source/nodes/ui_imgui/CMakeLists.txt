
USTC_CG_ADD_LIB(
	nodes_ui_imgui
	PUBLIC_LIBS GUI
	PRIVATE_LIBS glfw Logger stb_image nodes_core nodes_system RHI
	INC_DIR ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui-node-editor ${CMAKE_CURRENT_LIST_DIR}
)
