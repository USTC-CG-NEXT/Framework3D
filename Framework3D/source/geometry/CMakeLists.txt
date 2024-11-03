file(GLOB CPP_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

add_library(geometry ${CPP_FILES})

target_link_libraries(geometry PUBLIC OpenMeshCore)

target_include_directories(geometry PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(geometry PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_compile_options(geometry PRIVATE -DUSTC_CG_BUILD_MODULE=1 -DNOMINMAX)
set_target_properties(geometry PROPERTIES ${OUTPUT_DIR})

target_link_libraries(geometry 
	PUBLIC
	usdVol
	OpenMeshCore
	usdGeom 
	usdSkel
	usdFoam
	${TBB_tbb_LIBRARY}
)
target_compile_features(geometry PUBLIC cxx_std_20)

# add_subdirectory(usdFoam)