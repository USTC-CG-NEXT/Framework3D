#include "basic_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(iteration_end)
{
    b.add_input<std::string>("Name").default_val("Iteration");
    b.add_input<entt::meta_any>("iteration");
}

NODE_EXECUTION_FUNCTION(iteration_end)
{
    return true;
}

NODE_DECLARATION_FUNCTION(iteration_begin)
{
    b.add_input<std::string>("Name").default_val("Iteration");
    b.add_output<entt::meta_any>("iteration");
}

NODE_EXECUTION_FUNCTION(iteration_begin)
{
    return true;
}



NODE_DECLARATION_UI(iteration);
NODE_DEF_CLOSE_SCOPE
