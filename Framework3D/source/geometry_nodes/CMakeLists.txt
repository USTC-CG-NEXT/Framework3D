add_nodes(
	TARGET_NAME geometry_nodes 
	CONVERSION_FILES conversion/node_conversion.cpp
	DEP_LIBS stage nodes_system usd geometry usdShade
	COMPILE_DEFS NOMINMAX 
)