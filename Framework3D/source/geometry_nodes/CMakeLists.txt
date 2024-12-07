add_nodes(
	TARGET_NAME geometry_nodes 
	DEP_LIBS stage nodes_system usd geometry usdShade Eigen3::Eigen autodiff
	COMPILE_DEFS NOMINMAX 
)