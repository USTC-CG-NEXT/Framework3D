#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(simulation_in)
{
    b.add_input_group("Simulation In");
    b.add_output_group("Simulation Out");
}

NODE_EXECUTION_FUNCTION(simulation_in)
{
    auto inputs = params.get_input_group("Simulation In");

    std::vector<entt::meta_any> outputs;

    for (auto& input : inputs) {
        outputs.push_back(*input);
    }

    params.set_output_group("Simulation Out", outputs);

    return true;
}

NODE_DECLARATION_FUNCTION(simulation_out)
{
    b.add_input_group("Simulation In").set_runtime_dynamic(false);
    b.add_output_group("Simulation Out").set_runtime_dynamic(false);
}

NODE_EXECUTION_FUNCTION(simulation_out)
{
    auto inputs = params.get_input_group("Simulation In");

    std::vector<entt::meta_any> outputs;

    for (auto& input : inputs) {
        outputs.push_back(*input);
    }

    params.set_output_group("Simulation Out", outputs);
    return true;
}

NODE_DEF_CLOSE_SCOPE