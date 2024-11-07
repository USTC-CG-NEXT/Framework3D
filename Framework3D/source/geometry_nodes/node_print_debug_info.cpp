#include <fstream>

#include "geom_node_base.h"
#include "macro_map.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(print_debug_info)
{
    b.add_input<entt::meta_any>("Variable");
}

#define TypesToPrint float, VtArray<float>

NODE_EXECUTION_FUNCTION(print_debug_info)
{
    entt::meta_any storage = params.get_input<entt::meta_any>("Variable");
    using namespace pxr;
#define PrintType(type)               \
    if (storage.allow_cast<type>()) { \
        std::ostringstream out;       \
        out << storage.cast<type>();  \
        log::info(out.str().c_str()); \
    }

    MACRO_MAP(PrintType, TypesToPrint);
    return true;
}

NODE_DECLARATION_REQUIRED(print_debug_info);

NODE_DECLARATION_UI(print_debug_info);
NODE_DEF_CLOSE_SCOPE
