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
#define PrintType(type)                                 \
    if (storage.allow_cast<type>()) {                   \
        std::cout << storage.cast<type>() << std::endl; \
    }

    MACRO_MAP(PrintType, TypesToPrint)

    auto stage = params.get_global_payload<GeomPayload>().stage;
    std::string str;
    stage->ExportToString(&str);

    std::ofstream out("current_stage.usda");
    out << str << std::endl;
}

NODE_DECLARATION_UI(print_debug_info);
NODE_DEF_CLOSE_SCOPE
