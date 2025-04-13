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
# add back the renderer
add_dependencies(USTC_CG_test hd_USTC_CG)
add_dependencies(USTC_CG_test render_nodes)
add_dependencies(USTC_CG_test gl_based_render_nodes)
add_dependencies(USTC_CG_test hd_USTC_CG_Embree)


add_dependencies(USTC_CG_polyscope_test geometry_nodes)
add_dependencies(USTC_CG_polyscope_test basic_nodes)
add_dependencies(USTC_CG_polyscope_test polyscope_nodes)
add_dependencies(USTC_CG_polyscope_test optimization)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../submissions_inner)
    message(STATUS "[Inner submissions found]")

add_dependencies(USTC_CG_polyscope_test submissions_inner_nodes)
add_dependencies(USTC_CG_test submissions_inner_nodes)
endif ()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../../submissions)
    message(STATUS "[Outer submissions found]")
add_dependencies(USTC_CG_polyscope_test submissions_nodes)
add_dependencies(USTC_CG_test submissions_nodes)
endif ()
