
function(Set_CUDA_Properties lib_name)
    set_target_properties(${lib_name}
        PROPERTIES
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
        CUDA_SEPARABLE_COMPILATION ON
    )

    target_compile_features(${lib_name} PUBLIC cuda_std_20)
    target_compile_options(${lib_name}
        PUBLIC
        "$<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr;--extended-lambda;--forward-unknown-to-host-compiler;-g;-lineinfo;-rdc=true;-c;>"
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
function(USTC_CG_ADD_LIB LIB_NAME)
    set(options SHARED)
    set(oneValueArgs SRC_DIR)
    set(multiValueArgs LIB_FLAGS EXTRA_FILES INC_DIR PUBLIC_LIBS PRIVATE_LIBS COMPILE_OPTIONS)
    cmake_parse_arguments(USTC_CG_ADD_LIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(name ${LIB_NAME})

    set(folder ${USTC_CG_ADD_LIB_SRC_DIR})
    if(NOT USTC_CG_ADD_LIB_SRC_DIR)
        set(folder ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    file(GLOB_RECURSE ${name}_src_headers ${folder}/*.h)
    file(GLOB_RECURSE ${name}_cpp_sources ${folder}/*.cpp)

    set(${name}_sources
        ${${name}_cpp_sources}
        ${${name}_headers}
        ${${name}_src_headers}
        ${USTC_CG_ADD_LIB_EXTRA_FILES}
    )

    if(USTC_CG_WITH_CUDA)
        file(GLOB_RECURSE ${name}_cuda_sources ${folder}/*.cu ${folder}/*.cuh)
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

    target_link_libraries(${name} 
        PUBLIC ${USTC_CG_ADD_LIB_PUBLIC_LIBS}
        PRIVATE ${USTC_CG_ADD_LIB_PRIVATE_LIBS}
    )
endfunction()
