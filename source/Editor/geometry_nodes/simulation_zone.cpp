#include "geom_node_base.h"

struct SimulationStorage {
    std::vector<entt::meta_any> data;
    static constexpr bool has_storage = false;
};

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(simulation_in)
{
    b.add_input_group("Simulation In");
    b.add_output_group("Simulation Out");
}

NODE_EXECUTION_FUNCTION(simulation_in)
{
    auto& global_payload = params.get_global_payload<GeomPayload&>();
    global_payload.has_simulation = true;

    std::vector<entt::meta_any*> inputs;
    if (!global_payload.is_simulating) {
        inputs = params.get_input_group("Simulation In");
        std::vector<entt::meta_any> outputs;

        for (auto& input : inputs) {
            outputs.push_back(std::move(*input));
        }

        params.set_output_group("Simulation Out", std::move(outputs));
    }
    else {
        auto& outputs = params.get_storage<SimulationStorage&>().data;
        params.set_output_group("Simulation Out", std::move(outputs));
    }

    return true;
}

NODE_DECLARATION_FUNCTION(simulation_out)
{
    b.add_input_group("Simulation In");
    b.add_output_group("Simulation Out");
}

NODE_EXECUTION_FUNCTION(simulation_out)
{
    auto inputs = params.get_input_group("Simulation In");

    std::vector<entt::meta_any> outputs;

    for (auto& input : inputs) {
        outputs.push_back(std::move(*input));
    }
    params.get_storage<SimulationStorage&>().data = outputs;
    params.set_output_group("Simulation Out", std::move(outputs));
    return true;
}

NODE_DEF_CLOSE_SCOPE