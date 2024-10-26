
#include "all_socket_types.hpp"
#include "node.hpp"
#include "socket_types/geo_socket_types.hpp"
#include "socket_types/render_socket_types.hpp"
#include "socket_types/stage_socket_types.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE


#define BUILD_TYPE(NAME)                                             \
    NodeSocket* decl::NAME::build(NodeTree* ntree, Node* node) const \
    {                                                                \
        NodeSocket* socket = node->add_socket(                       \
            type->type_name().c_str(),                               \
            this->identifier.c_str(),                                \
            this->name.c_str(),                                      \
            this->in_out);                                           \
        update_default_value(socket);                                \
                                                                     \
        return socket;                                               \
    }

// MACRO_MAP(BUILD_TYPE, ALL_SOCKET_TYPES)

NodeDeclarationBuilder::NodeDeclarationBuilder(NodeDeclaration& declaration)
    : declaration_(declaration)
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
