add_nodes(
    TARGET_NAME polyscope_nodes
    DEP_LIBS nodes_system polyscope_widget geometry
    COMPILE_DEFS NOMINMAX
)