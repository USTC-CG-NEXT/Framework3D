add_nodes(
	TARGET_NAME basic_nodes 
	CONVERSION_FILES conversion/node_conversion.cpp
	DEP_LIBS stage nodes_system usd usdShade
	COMPILE_DEFS NOMINMAX 
)