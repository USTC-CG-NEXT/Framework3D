add_nodes(
	TARGET_NAME optimization
	DEP_LIBS stage nodes_system usd geometry usdShade Eigen3::Eigen autodiff
	COMPILE_DEFS NOMINMAX 
)