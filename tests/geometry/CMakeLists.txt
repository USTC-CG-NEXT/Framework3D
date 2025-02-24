file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
foreach(source ${test_sources})
    UCG_ADD_TEST(
        SRC ${source} 
        LIBS OpenMeshCore Eigen3::Eigen nodes_core
	)
endforeach()
