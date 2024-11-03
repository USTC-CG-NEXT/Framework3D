#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE
// Through one execution, how much time is advected? Unit is seconds.
NODE_DECLARATION_FUNCTION(declare_time_gain)
{
    b.add_input<float>("time")
        .default_val(0.0333333333f)
        .min(0)
        .max(0.2f);
}

NODE_EXECUTION_FUNCTION(exec_time_gain)
{
    // This is for external read. Do nothing.
}

// Through one execution, how much time is advected? Unit is seconds.
NODE_DECLARATION_FUNCTION(declare_time_code)
{
    b.add_output<float>("time");
}

NODE_EXECUTION_FUNCTION(exec_time_code)
{
    // This is for external write. Do nothing.
}



NODE_DECLARATION_UI(time_code);
NODE_DEF_CLOSE_SCOPE
