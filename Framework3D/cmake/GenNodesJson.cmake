include(CMakeParseArguments)

function(GEN_NODES_JSON TARGET_NAME)
    set(options)
    set(oneValueArgs OUTPUT_JSON)
    set(multiValueArgs NODES_DIRS CONVERSIONS_DIRS)
    cmake_parse_arguments(GEN_NODES_JSON "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message("GEN_NODES_JSON at ${GEN_NODES_JSON_OUTPUT_JSON}")

    # Convert each directory path to absolute if it is not already
    foreach(dir IN LISTS GEN_NODES_JSON_NODES_DIRS)
        if(NOT IS_ABSOLUTE "${dir}")
            get_filename_component(dir "${dir}" ABSOLUTE)
        endif()
        list(APPEND ABS_NODES_DIRS "${dir}")
    endforeach()

    foreach(dir IN LISTS GEN_NODES_JSON_CONVERSIONS_DIRS)
        if(NOT IS_ABSOLUTE "${dir}")
            get_filename_component(dir "${dir}" ABSOLUTE)
        endif()
        list(APPEND ABS_CONVERSIONS_DIRS "${dir}")
    endforeach()

    # Convert the list of directories to a semicolon-separated string
    string(REPLACE ";" " " NODES_DIRS_STR "${ABS_NODES_DIRS}")
    string(REPLACE ";" " " CONVERSIONS_DIRS_STR "${ABS_CONVERSIONS_DIRS}")

    # Construct the command to call the Python script
    set(COMMAND_ARGS ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/source/util_scripts/nodes_json.py)
    
    if(NODES_DIRS_STR)
        list(APPEND COMMAND_ARGS --nodes ${NODES_DIRS_STR})
    endif()
    
    if(CONVERSIONS_DIRS_STR)
        list(APPEND COMMAND_ARGS --conversions ${CONVERSIONS_DIRS_STR})
    endif()
    
    list(APPEND COMMAND_ARGS --output ${GEN_NODES_JSON_OUTPUT_JSON})

    add_custom_command(
        OUTPUT ${GEN_NODES_JSON_OUTPUT_JSON}
        COMMAND ${COMMAND_ARGS}
        DEPENDS ${ABS_NODES_DIRS} ${ABS_CONVERSIONS_DIRS}
        COMMENT "Generating JSON file with node and conversion information"
    )

    add_custom_target(
        ${TARGET_NAME} ALL
        DEPENDS ${GEN_NODES_JSON_OUTPUT_JSON}
    )

endfunction()