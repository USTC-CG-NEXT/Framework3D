
USTC_CG_ADD_LIB(GUI 
	INC_DIR 
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui-node-editor 
	${CMAKE_BINARY_DIR}/include 
	${PROJECT_SOURCE_DIR}/include
	PUBLIC_LIBS imgui nodes_core
)


# file(GLOB_RECURSE GUI_source ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# 
# add_library(GUI STATIC ${GUI_source})
# 
# target_link_libraries(GUI PUBLIC 
# 	imgui 
# 	# stb_image 
# 	# GCore
# 	nodes_core
# 	# RCore
# 	# usdImaging
# 	# usdImagingGL
# 	# Python3::Python
# 	# usdFoam
# )
# target_compile_options(GUI PRIVATE -DUSTC_CG_BUILD_MODULE=0  -DNOMINMAX)
# 
# target_include_directories(GUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 
# 
# set_target_properties(GUI PROPERTIES ${OUTPUT_DIR})
