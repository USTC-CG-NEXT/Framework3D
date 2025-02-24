#include "Logger/Logger.h"
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_UI(print)
{
    return "Print Info";
}

NODE_DECLARATION_FUNCTION(print)
{
    b.add_input<int>("info").min(0).max(10).default_val(1);
}

NODE_EXECUTION_FUNCTION(print)
{
    auto val = params.get_input<int>("info");

    std::ostringstream oss;
    oss << "Print Info: " << val;
    log::info(oss.str().c_str());
    return true;
}

NODE_DECLARATION_REQUIRED(print)
NODE_DEF_CLOSE_SCOPE
