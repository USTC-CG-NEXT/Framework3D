#include "basic_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(storage_in)
{
    b.add_input<std::string>("Name").default_val("Storage");
    b.add_input<entt::meta_any>("storage");
}

NODE_EXECUTION_FUNCTION(storage_in)
{
    return true;
}

NODE_DECLARATION_FUNCTION(storage_out)
{
    b.add_input<std::string>("Name").default_val("Storage");
    b.add_output<entt::meta_any>("storage");
}

NODE_EXECUTION_FUNCTION(storage_out)
{
    return true;
}

NODE_DECLARATION_UI(storage);
NODE_DEF_CLOSE_SCOPE
