
#include "nvrhi/nvrhi.h"
#include "nvrhi/utils.h"
#include "render_node_base.h"


#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(test_socket)
{
    b.add_input<nvrhi::TextureHandle>("tt");
    b.add_output<nvrhi::TextureHandle>("tt");
}

NODE_EXECUTION_FUNCTION(test_socket)
{
    return true;

}



NODE_DECLARATION_UI(test_socket);
NODE_DEF_CLOSE_SCOPE
