add_nodes(
	TARGET_NAME geometry_nodes 
	DEP_LIBS stage nodes_system usd geometry usdShade Eigen3::Eigen autodiff igl::core igl_restricted::triangle
	COMPILE_DEFS NOMINMAX 
)