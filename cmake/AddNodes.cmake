include(CMakeParseArguments)

function(GEN_NODES_JSON TARGET_NAME)
    set(options)
    set(oneValueArgs OUTPUT_JSON)
    set(multiValueArgs NODES_DIRS NODES_FILES CONVERSIONS_DIRS CONVERSIONS_FILES)
    cmake_parse_arguments(GEN_NODES_JSON "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message("GEN_NODES_JSON at ${GEN_NODES_JSON_OUTPUT_JSON}")

    # Convert each directory path to absolute if it is not already
    foreach(dir IN LISTS GEN_NODES_JSON_NODES_DIRS)
        if(NOT IS_ABSOLUTE "${dir}")
            get_filename_component(dir "${dir}" ABSOLUTE)
        endif()
        list(APPEND ABS_NODES_DIRS "${dir}")
    endforeach()

    foreach(file IN LISTS GEN_NODES_JSON_NODES_FILES)
        if(NOT IS_ABSOLUTE ("${file}"))
            get_filename_component(file "${file}" ABSOLUTE)
        endif()
        list(APPEND ABS_NODES_FILES "${file}")
    endforeach()

    foreach(dir IN LISTS GEN_NODES_JSON_CONVERSIONS_DIRS)
        if(NOT IS_ABSOLUTE ("${dir}"))
            get_filename_component(dir "${dir}" ABSOLUTE)
        endif()
        list(APPEND ABS_CONVERSIONS_DIRS "${dir}")
    endforeach()

    foreach(file IN LISTS GEN_NODES_JSON_CONVERSIONS_FILES)
        if(NOT IS_ABSOLUTE ("${file}"))
            get_filename_component(file "${file}" ABSOLUTE)
        endif()
        list(APPEND ABS_CONVERSIONS_FILES "${file}")
    endforeach()

    # Convert the list of directories and files to a semicolon-separated string
    string(REPLACE ";" " " NODES_DIRS_STR "${ABS_NODES_DIRS}")
    string(REPLACE ";" " " NODES_FILES_STR "${ABS_NODES_FILES}")
    string(REPLACE ";" " " CONVERSIONS_DIRS_STR "${ABS_CONVERSIONS_DIRS}")
    string(REPLACE ";" " " CONVERSIONS_FILES_STR "${ABS_CONVERSIONS_FILES}")

    # Construct the command to call the Python script
    set(COMMAND_ARGS ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/source/Plugins/util_scripts/nodes_json.py)

    if(NODES_DIRS_STR)
        list(APPEND COMMAND_ARGS --nodes-dir ${NODES_DIRS_STR})
    endif()

    if(NODES_FILES_STR)
        list(APPEND COMMAND_ARGS --nodes-files ${NODES_FILES_STR})
    endif()

    if(CONVERSIONS_DIRS_STR)
        list(APPEND COMMAND_ARGS --conversions-dir ${CONVERSIONS_DIRS_STR})
    endif()

    if(CONVERSIONS_FILES_STR)
        list(APPEND COMMAND_ARGS --conversions-files ${CONVERSIONS_FILES_STR})
    endif()

    list(APPEND COMMAND_ARGS --output ${GEN_NODES_JSON_OUTPUT_JSON})

    message("COMMAND_ARGS: ${COMMAND_ARGS}")

    add_custom_command(
        OUTPUT ${GEN_NODES_JSON_OUTPUT_JSON}
        COMMAND ${COMMAND_ARGS}
        DEPENDS ${ABS_NODES_DIRS} ${ABS_NODES_FILES} ${ABS_CONVERSIONS_DIRS} ${ABS_CONVERSIONS_FILES}
        COMMENT "Generating JSON file with node and conversion information"
    )

    add_custom_target(
        ${TARGET_NAME} ALL
        DEPENDS ${GEN_NODES_JSON_OUTPUT_JSON}
    )

endfunction()

function(add_nodes)
    cmake_parse_arguments(ARG "" "TARGET_NAME" "SRC_DIRS;SRC_FILES;CONVERSION_DIRS;CONVERSION_FILES;DEP_LIBS;COMPILE_DEFS;COMPILE_OPTIONS;EXTRA_INCLUDE_DIRS" ${ARGN})

    if(NOT ARG_TARGET_NAME)
        message(FATAL_ERROR "add_nodes: TARGET_NAME not specified")
    endif()

    if(NOT ARG_SRC_DIRS)
        set(ARG_SRC_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    foreach(src_dir IN LISTS ARG_SRC_DIRS)
        file(GLOB src_files ${src_dir}/*.cpp)
        list(APPEND ARG_SRC_FILES_IN_DIRS ${src_files})
    endforeach()

    foreach(conversion_dir IN LISTS ARG_CONVERSION_DIRS)
        file(GLOB conversion_files ${conversion_dir}/*.cpp)
        list(APPEND ARG_CONVERSION_IN_DIRS ${conversion_files})
    endforeach()


    set(ALL_THAT_NEEDS_TO_BE_COMPILED ${ARG_SRC_FILES_IN_DIRS} ${ARG_CONVERSION_IN_DIRS} ${ARG_SRC_FILES} ${ARG_CONVERSION_FILES})

    foreach(source ${ALL_THAT_NEEDS_TO_BE_COMPILED})
        get_filename_component(target_name ${source} NAME_WE)
        add_library(${target_name} MODULE ${source})
        set_target_properties(${target_name} PROPERTIES ${OUTPUT_DIR})
        target_link_libraries(${target_name} PUBLIC nodes_core ${ARG_DEP_LIBS})
        if(ARG_COMPILE_DEFS)
            target_compile_definitions(${target_name} PRIVATE ${ARG_COMPILE_DEFS})
        endif()
        if(ARG_COMPILE_OPTIONS)
            target_compile_options(${target_name} PRIVATE ${ARG_COMPILE_OPTIONS})
        endif()
        if(ARG_EXTRA_INCLUDE_DIRS)
            target_include_directories(${target_name} PRIVATE ${ARG_EXTRA_INCLUDE_DIRS})
        endif()
        list(APPEND all_nodes ${target_name})
    endforeach()


    GEN_NODES_JSON(${ARG_TARGET_NAME}_json_target
        NODES_DIRS ${ARG_SRC_DIRS}
        NODES_FILES ${ARG_SRC_FILES}
        CONVERSIONS_DIRS ${ARG_CONVERSION_DIRS}
        CONVERSIONS_FILES ${ARG_CONVERSION_FILES}
        OUTPUT_JSON ${OUT_BINARY_DIR}/${ARG_TARGET_NAME}.json
    )

    add_library(${ARG_TARGET_NAME} INTERFACE)
    add_dependencies(${ARG_TARGET_NAME} ${all_nodes} ${all_conversions} ${ARG_TARGET_NAME}_json_target)
endfunction()
