#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE
// Through one execution, how much time is advected? Unit is seconds.
NODE_DECLARATION_FUNCTION(time_gain)
{
    b.add_input<float>("time").default_val(0.0333333333f).min(0).max(0.2f);
}

NODE_EXECUTION_FUNCTION(time_gain)
{
    // This is for external read. Do nothing.
    return true;
}

// Through one execution, how much time is advected? Unit is seconds.
NODE_DECLARATION_FUNCTION(time_code)
{
    b.add_output<float>("time");
}

NODE_EXECUTION_FUNCTION(time_code)
{
    // This is for external write. Do nothing.
    auto& global_payload = params.get_global_payload<GeomPayload&>();
    global_payload.has_simulation = true;
    auto current_time = global_payload.current_time;
    params.set_output("time", (float)current_time.GetValue());
    return true;
}
NODE_DECLARATION_REQUIRED(time_code)

NODE_DEF_CLOSE_SCOPE
