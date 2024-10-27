file(GLOB_RECURSE
  CPP_CORE_FILES
  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

add_library(nodes_system STATIC ${CPP_CORE_FILES})
target_link_libraries(nodes_system PUBLIC nodes_core)