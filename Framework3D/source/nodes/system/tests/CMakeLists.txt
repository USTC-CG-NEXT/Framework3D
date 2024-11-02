add_library(node_add SHARED test_node/node_add.cpp)
set_target_properties(node_add PROPERTIES ${OUTPUT_DIR})
target_link_libraries(node_add PRIVATE nodes_core)

add_library(node_sub SHARED test_node/node_sub.cpp)
set_target_properties(node_sub PROPERTIES ${OUTPUT_DIR})
target_link_libraries(node_sub PRIVATE nodes_core)

set(all_nodes node_add node_sub)


GEN_NODES_JSON(test_node_json_target NODES_DIRS test_node OUTPUT_JSON ${OUT_BINARY_DIR}/test_nodes.json)

add_library(test_nodes INTERFACE)
add_dependencies(test_nodes node_add node_sub test_node_json_target)

add_dependencies(node_system_test test_nodes)