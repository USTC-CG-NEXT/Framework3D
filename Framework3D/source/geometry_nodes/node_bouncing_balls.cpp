#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(bouncing)
{
    b.add_input<float>("p_z");
    b.add_input<float>("v_z");
    b.add_input<float>("k").default_val(1).min(0.2).max(20);
    b.add_input<float>("h");
    b.add_input<float>("time_code");

    b.add_output<float>("p_z");
    b.add_output<float>("v_z");
}

NODE_EXECUTION_FUNCTION(bouncing)
{
    auto time_code = params.get_input<float>("time_code");
    auto p_z = params.get_input<float>("p_z");
    auto v_z = params.get_input<float>("v_z");
    auto k = params.get_input<float>("k");
    auto h = params.get_input<float>("h");

    if (time_code == 0) {
        p_z = 1;
        v_z = 0;
        params.set_output("p_z", p_z);
        params.set_output("v_z", v_z);

        return true;
    }

    v_z += -h * p_z * k;
    p_z += h * v_z;

    params.set_output("p_z", p_z);
    params.set_output("v_z", v_z);

    return true;
}

NODE_DECLARATION_UI(bouncing);
NODE_DEF_CLOSE_SCOPE
