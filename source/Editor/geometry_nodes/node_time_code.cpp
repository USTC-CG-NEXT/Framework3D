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
    return true;
}

NODE_DEF_CLOSE_SCOPE
