add_library(test_node_add SHARED test_node/node_add.cpp)
set_target_properties(test_node_add PROPERTIES ${OUTPUT_DIR})
target_link_libraries(test_node_add PRIVATE nodes_core)

add_library(test_node_sub SHARED test_node/node_sub.cpp)
set_target_properties(test_node_sub PROPERTIES ${OUTPUT_DIR})
target_link_libraries(test_node_sub PRIVATE nodes_core)

set(all_nodes test_node_add test_node_sub)

function(generate_json_file JSON_FILE_NAME)
    cmake_parse_arguments(ARG "" "" "NODES" ${ARGN})

    set(JSON_FILE_CONTENT "{\n")

    foreach(node ${ARG_NODES})
        if(WIN32)
            set(SUFFIX "dll")
        elseif(UNIX)
            set(SUFFIX "so")
        else()
            set(SUFFIX "unknown")
        endif()

        set(JSON_FILE_CONTENT "${JSON_FILE_CONTENT}  \"${node}\": \"${node}.${SUFFIX}\",\n")
    endforeach()

    # Remove the last comma and add closing brace
    string(REGEX REPLACE ",\n$" "\n" JSON_FILE_CONTENT "${JSON_FILE_CONTENT}")
    set(JSON_FILE_CONTENT "${JSON_FILE_CONTENT}}")

    file(WRITE ${OUT_BINARY_DIR}/${JSON_FILE_NAME}.json ${JSON_FILE_CONTENT})
endfunction()

generate_json_file(test_nodes NODES ${all_nodes})

add_library(test_nodes INTERFACE)
add_dependencies(test_nodes test_node_add test_node_sub)

add_dependencies(node_system_test test_nodes)