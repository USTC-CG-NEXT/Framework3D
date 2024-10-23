file(GLOB_RECURSE
  CPP_CORE_FILES
  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

add_library(nodes_core STATIC ${CPP_CORE_FILES})
target_link_libraries(nodes_core PRIVATE EnTT::EnTT)

target_include_directories(nodes_core PUBLIC ${PROJECT_SOURCE_DIR}/include)