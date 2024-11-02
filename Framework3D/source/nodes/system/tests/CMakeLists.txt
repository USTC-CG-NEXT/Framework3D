file(GLOB TEST_NODE_SOURCES test_node/*.cpp)

foreach(source ${TEST_NODE_SOURCES})
    get_filename_component(target_name ${source} NAME_WE)
    add_library(${target_name} SHARED ${source})
    set_target_properties(${target_name} PROPERTIES ${OUTPUT_DIR})
    target_link_libraries(${target_name} PRIVATE nodes_core)
    list(APPEND all_nodes ${target_name})
endforeach()

GEN_NODES_JSON(test_node_json_target 
    NODES_DIRS test_node 
    OUTPUT_JSON ${OUT_BINARY_DIR}/test_nodes.json
)

add_library(test_nodes INTERFACE)
add_dependencies(test_nodes ${all_nodes} test_node_json_target)

add_dependencies(node_system_test test_nodes)