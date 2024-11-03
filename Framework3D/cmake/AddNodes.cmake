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

function(add_nodes)
    cmake_parse_arguments(ARG "" "SRC_DIR;TARGET_NAME;CONVERSION_DIR" "SRC_FILES;CONVERSION_FILES;DEP_LIBS;COMPILE_DEFS;COMPILE_OPTIONS" ${ARGN})

    if(NOT ARG_SRC_DIR)
        set(ARG_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
        message("SRC_DIR is not set. Using ${ARG_SRC_DIR}")
    endif()

    if(NOT ARG_SRC_FILES)
        file(GLOB ARG_SRC_FILES ${ARG_SRC_DIR}/*.cpp)
    endif()

    if(NOT ARG_CONVERSION_DIR)
        message("CONVERSION_DIR is not set.")
    endif()

    if(NOT ARG_CONVERSION_FILES)
        file(GLOB ARG_CONVERSION_FILES ${ARG_CONVERSION_DIR}/*.cpp)
    endif()

    set(SRC_DIRS ${ARG_SRC_DIR})
    set(CONVERSION_DIRS ${ARG_CONVERSION_DIR})

    foreach(source ${ARG_SRC_FILES})
        get_filename_component(source_dir ${source} DIRECTORY)
        list(FIND SRC_DIRS ${source_dir} src_dir_index)
        if(src_dir_index EQUAL -1)
            list(APPEND SRC_DIRS ${source_dir})
        endif()
        get_filename_component(target_name ${source} NAME_WE)
        add_library(${target_name} SHARED ${source})
        set_target_properties(${target_name} PROPERTIES ${OUTPUT_DIR})
        target_link_libraries(${target_name} PUBLIC nodes_core ${ARG_DEP_LIBS})
        if(ARG_COMPILE_DEFS)
            target_compile_definitions(${target_name} PRIVATE ${ARG_COMPILE_DEFS})
        endif()
        if(ARG_COMPILE_OPTIONS)
            target_compile_options(${target_name} PRIVATE ${ARG_COMPILE_OPTIONS})
        endif()
        list(APPEND all_nodes ${target_name})
    endforeach()

    foreach(source ${ARG_CONVERSION_FILES})
        get_filename_component(source_dir ${source} DIRECTORY)
        list(FIND CONVERSION_DIRS ${source_dir} conversion_dir_index)
        if(conversion_dir_index EQUAL -1)
            list(APPEND CONVERSION_DIRS ${source_dir})
        endif()
        get_filename_component(target_name ${source} NAME_WE)
        add_library(${target_name} SHARED ${source})
        set_target_properties(${target_name} PROPERTIES ${OUTPUT_DIR})
        target_link_libraries(${target_name} PUBLIC nodes_core ${ARG_DEP_LIBS})
        if(ARG_COMPILE_DEFS)
            target_compile_definitions(${target_name} PRIVATE ${ARG_COMPILE_DEFS})
        endif()
        if(ARG_COMPILE_OPTIONS)
            target_compile_options(${target_name} PRIVATE ${ARG_COMPILE_OPTIONS})
        endif()
        list(APPEND all_conversions ${target_name})
    endforeach()

    GEN_NODES_JSON(${ARG_TARGET_NAME}_json_target 
        NODES_DIRS ${SRC_DIRS} 
        CONVERSIONS_DIRS ${CONVERSION_DIRS}
        OUTPUT_JSON ${OUT_BINARY_DIR}/${ARG_TARGET_NAME}.json
    )

    add_library(${ARG_TARGET_NAME} INTERFACE)
    add_dependencies(${ARG_TARGET_NAME} ${all_nodes} ${all_conversions} ${ARG_TARGET_NAME}_json_target)
endfunction()