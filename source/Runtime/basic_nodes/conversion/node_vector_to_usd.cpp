
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(vector_to_usd)
{
    // Function content omitted
    b.add_input<std::vector<std::vector<float>>>("Input vector");
}

NODE_EXECUTION_FUNCTION(vector_to_usd)
{
    // Function content omitted
    return true;
}

NODE_DECLARATION_REQUIRED(vector_to_usd)
NODE_DECLARATION_UI(vector_to_usd);
NODE_DEF_CLOSE_SCOPE
