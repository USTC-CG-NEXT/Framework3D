file(GLOB_RECURSE
  CPP_CORE_FILES
  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

add_library(nodes_ui_imgui STATIC ${CPP_CORE_FILES})
target_link_libraries(nodes_ui_imgui PUBLIC nodes_system GUI)
target_include_directories(nodes_ui_imgui PRIVATE ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui-node-editor)
