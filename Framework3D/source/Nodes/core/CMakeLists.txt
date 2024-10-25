file(GLOB_RECURSE
  CPP_CORE_FILES
  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

add_library(nodes_core STATIC ${CPP_CORE_FILES})
target_link_libraries(nodes_core PRIVATE EnTT::EnTT)
target_include_directories(nodes_core PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../include/internal)
target_include_directories(nodes_core PRIVATE ${PROJECT_SOURCE_DIR}/include/api)
target_compile_options(nodes_core PRIVATE -DUSTC_CG_BUILD_MODULE=1)
