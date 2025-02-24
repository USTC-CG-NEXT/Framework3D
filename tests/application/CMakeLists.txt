file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(source ${test_sources})
    UCG_ADD_TEST(
        SRC ${source}
        LIBS usdview_widget nodes_system GUI usd nodes_ui_imgui geometry polyscope_widget
    )
endforeach()

add_dependencies(USTC_CG_test geometry_nodes)
add_dependencies(USTC_CG_test basic_nodes)
add_dependencies(USTC_CG_test polyscope_nodes)
add_dependencies(USTC_CG_test optimization)
add_dependencies(USTC_CG_test hd_USTC_CG)