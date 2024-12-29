add_nodes(
    TARGET_NAME polyscope_nodes
    DEP_LIBS nodes_system polyscope_widget geometry igl::core igl_restricted::triangle
    COMPILE_DEFS NOMINMAX
)