
function(Set_CUDA_Properties lib_name)
    set_target_properties(${lib_name}
        PROPERTIES
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
        CUDA_SEPARABLE_COMPILATION ON
    )

    target_compile_features(${lib_name} PUBLIC cuda_std_20)
    target_compile_options(${lib_name}
        PUBLIC
        "$<<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr;--extended-lambda;--forward-unknown-to-host-compiler;-g;-lineinfo;-rdc=true;-c;>"
    )

    target_include_directories(
        ${lib_name}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(
        ${lib_name}
        PUBLIC
        #CUDA::cudart
        CUDA::nvrtc
    )
endfunction()

function(UCG_ADD_TEST)
    set(options SHARED)
    set(oneValueArgs SRC)
    set(multiValueArgs LIBS LIB_FLAGS EXTRA_FILES INCLUDE_DIRS)
    cmake_parse_arguments(UCG_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    string(REGEX REPLACE "(.*/)([a-zA-Z0-9_ ]+)(\.cpp|\.cu)" "\\2" test_name ${UCG_TEST_SRC})
    message("---- Adding the test ${test_name}_test.")

    add_executable(${test_name}_test ${UCG_TEST_SRC})

    set_target_properties(${test_name}_test PROPERTIES ${OUTPUT_DIR})

    get_target_property(gtest_include GTest::gtest_main INTERFACE_INCLUDE_DIRECTORIES)
    # There should be googletest available
    target_link_libraries(${test_name}_test PUBLIC gtest gtest_main)
    target_link_libraries(${test_name}_test PUBLIC ${UCG_TEST_LIBS})
    target_include_directories(${test_name}_test PUBLIC ${UCG_TEST_INCLUDE_DIRS})
    target_compile_definitions(${test_name}_test PUBLIC NOMINMAX=1)

    add_test(
        NAME
        ${test_name}_test
        COMMAND
        ${test_name}_test
    )
endfunction(UCG_ADD_TEST)

function(USTC_CG_ADD_LIB LIB_NAME)
    set(options SHARED)
    set(oneValueArgs SRC_DIR RESOURCE_COPY_TARGET)
    set(multiValueArgs LIB_FLAGS EXTRA_FILES INC_DIR PUBLIC_LIBS PRIVATE_LIBS COMPILE_OPTIONS COMPILE_DEFS USD_RESOURCE_DIRS USD_RESOURCE_FILES)
    cmake_parse_arguments(USTC_CG_ADD_LIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT LIB_NAME)
        get_filename_component(LIB_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    endif()

    set(name ${LIB_NAME})

    set(folder ${USTC_CG_ADD_LIB_SRC_DIR})
    if(NOT USTC_CG_ADD_LIB_SRC_DIR)
        set(folder ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    file(GLOB_RECURSE ${name}_src_headers ${folder}/*.h)
    file(GLOB_RECURSE ${name}_cpp_sources ${folder}/*.cpp)

    # Exclude files under ${SRC_DIR}/test and ${USD_RESOURCE_DIRS}
    list(FILTER ${name}_src_headers EXCLUDE REGEX "${folder}/tests/.*")
    list(FILTER ${name}_cpp_sources EXCLUDE REGEX "${folder}/tests/.*")
    foreach(resource_dir ${USTC_CG_ADD_LIB_USD_RESOURCE_DIRS})
        list(FILTER ${name}_src_headers EXCLUDE REGEX "${resource_dir}/.*")
        list(FILTER ${name}_cpp_sources EXCLUDE REGEX "${resource_dir}/.*")
    endforeach()

    set(${name}_sources
        ${${name}_cpp_sources}
        ${${name}_headers}
        ${${name}_src_headers}
        ${USTC_CG_ADD_LIB_EXTRA_FILES}
    )

    if(USTC_CG_WITH_CUDA)
        file(GLOB_RECURSE ${name}_cuda_sources ${folder}/*.cu ${folder}/*.cuh)
        list(FILTER ${name}_cuda_sources EXCLUDE REGEX "${folder}/tests/.*")
        foreach(resource_dir ${USTC_CG_ADD_LIB_USD_RESOURCE_DIRS})
            list(FILTER ${name}_cuda_sources EXCLUDE REGEX "${resource_dir}/.*")
        endforeach()
        set(${name}_sources ${${name}_sources} ${${name}_cuda_sources})
        list(LENGTH ${name}_cuda_sources cuda_file_count)
    endif()

    if(${USTC_CG_ADD_LIB_SHARED})
        add_library(${name} SHARED ${USTC_CG_ADD_LIB_LIB_FLAGS} ${${name}_sources})
    else()
        add_library(${name} STATIC ${USTC_CG_ADD_LIB_LIB_FLAGS} ${${name}_sources})
    endif()
    target_compile_features(${name} PUBLIC cxx_std_20)

    target_include_directories(
        ${name}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${USTC_CG_ADD_LIB_INC_DIR}
    )

    if(USTC_CG_WITH_CUDA)
        if(NOT cuda_file_count EQUAL 0)
            Set_CUDA_Properties(${name})
        endif()
    endif()

    target_compile_options(${name} PRIVATE ${USTC_CG_ADD_LIB_COMPILE_OPTIONS})
    target_compile_definitions(${name} PRIVATE ${USTC_CG_ADD_LIB_COMPILE_DEFS})
    string(TOUPPER ${LIB_NAME} LIB_NAME_UPPER)
    target_compile_definitions(${name} PRIVATE BUILD_${LIB_NAME_UPPER}_MODULE=1)

    target_link_libraries(${name}
        PUBLIC ${USTC_CG_ADD_LIB_PUBLIC_LIBS}
        PRIVATE ${USTC_CG_ADD_LIB_PRIVATE_LIBS}
    )
    set_target_properties(${name} PROPERTIES ${OUTPUT_DIR})

    file(GLOB test_sources ${folder}/tests/*.cpp)
    foreach(source ${test_sources})
        UCG_ADD_TEST(
            SRC ${source}
            LIBS
            ${name}
            ${USTC_CG_ADD_LIB_PUBLIC_LIBS}
            ${USTC_CG_ADD_LIB_PRIVATE_LIBS}
        )
    endforeach()
    # Ensure the copy target directory exists only if RESOURCE_COPY_TARGET is specified
    if(USTC_CG_ADD_LIB_RESOURCE_COPY_TARGET)
        file(MAKE_DIRECTORY ${USTC_CG_ADD_LIB_RESOURCE_COPY_TARGET})
    else()
        set(USTC_CG_ADD_LIB_RESOURCE_COPY_TARGET ${OUT_BINARY_DIR}/usd)
        file(MAKE_DIRECTORY ${USTC_CG_ADD_LIB_RESOURCE_COPY_TARGET})
    endif()

    # Copy USD resource directories and files
    foreach(resource_dir ${USTC_CG_ADD_LIB_USD_RESOURCE_DIRS})
        get_filename_component(absolute_resource_dir ${resource_dir} ABSOLUTE)
        message("Copying USD resource directory ${absolute_resource_dir} to ${USTC_CG_ADD_LIB_RESOURCE_COPY_TARGET}")
        add_custom_command(
            TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${absolute_resource_dir} ${USTC_CG_ADD_LIB_RESOURCE_COPY_TARGET}
            COMMENT "Copying USD resource directory ${absolute_resource_dir} to ${USTC_CG_ADD_LIB_RESOURCE_COPY_TARGET}"
        )
    endforeach()

    foreach(resource_file ${USTC_CG_ADD_LIB_USD_RESOURCE_FILES})
        get_filename_component(absolute_resource_file ${resource_file} ABSOLUTE)
        add_custom_command(
            TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${absolute_resource_file} ${USTC_CG_ADD_LIB_RESOURCE_COPY_TARGET}
            COMMENT "Copying USD resource file ${absolute_resource_file} to ${USTC_CG_ADD_LIB_RESOURCE_COPY_TARGET}"
        )
    endforeach()
endfunction()
