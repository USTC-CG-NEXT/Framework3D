
USTC_CG_ADD_LIB(
	nodes_ui_imgui SHARED
	PUBLIC_LIBS GUI nodes_core
	PRIVATE_LIBS glfw Logger stb_image nodes_system RHI test_nodes
	INC_DIR ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui-node-editor ${CMAKE_CURRENT_LIST_DIR}
)
