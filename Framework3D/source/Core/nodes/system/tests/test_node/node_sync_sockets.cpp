

#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(simulation_in)
{
    b.add_input_group("Simulation In");
    b.add_output_group("Simulation Out");
}

NODE_EXECUTION_FUNCTION(simulation_in)
{
    return true;
}

NODE_DECLARATION_FUNCTION(simulation_out)
{
    b.add_input_group("Simulation In");
    b.add_output_group("Simulation Out");
}

NODE_EXECUTION_FUNCTION(simulation_out)
{
    return true;
}

NODE_DEF_CLOSE_SCOPE