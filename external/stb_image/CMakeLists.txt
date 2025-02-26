project(stb_image)

add_library(stb_image STATIC stb/stb_impl.cpp)

set(_stb_image_Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image_resize2.h
)

set_target_properties(stb_image PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(stb_image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stb_image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)
target_sources(stb_image INTERFACE ${_stb_image_Sources})

source_group("ThirdParty\\stb_image" FILES ${_stb_image_Sources})
