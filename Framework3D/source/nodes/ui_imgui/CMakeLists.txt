
USTC_CG_ADD_LIB(
	nodes_ui_imgui
	PUBLIC_LIBS nodes_system GUI
	PRIVATE_LIBS glfw imgui Logger
	INC_DIR ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui-node-editor
)
