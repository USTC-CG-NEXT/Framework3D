
#include "nvrhi/nvrhi.h"
#include "nvrhi/utils.h"
#include "render_node_base.h"


#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(dynamic_socket)
{
}

NODE_EXECUTION_FUNCTION(dynamic_socket)
{
    return true;
}



NODE_DECLARATION_UI(dynamic_socket);
NODE_DEF_CLOSE_SCOPE
