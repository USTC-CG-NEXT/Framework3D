#include "node_declare.hpp"

#include "all_socket_types.hpp"
#include "node.hpp"
#include "nodes.hpp"
#include "socket_types/geo_socket_types.hpp"
#include "socket_types/render_socket_types.hpp"
#include "socket_types/stage_socket_types.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

void decl::Int::update_default_value(NodeSocket* socket) const
{
    if (!socket->dataField.value) {
        socket->dataField.min = soft_min;
        socket->dataField.max = soft_max;
        socket->dataField.value =
            std::max(std::min(default_value_, soft_max), soft_min);
    }
}

void decl::Float::update_default_value(NodeSocket* socket) const
{
    if (!socket->dataField.value) {
        // TODO: When shall we destroy these?
        auto* default_value = &(socket->dataField);
        default_value->min = soft_min;
        default_value->max = soft_max;
        default_value->value =
            std::max(std::min(default_value_, soft_max), soft_min);
    }
}

void decl::String::update_default_value(NodeSocket* socket) const
{
    if (!socket->dataField.value) {
        socket->dataField.value = std::string(256, 0);
        memcpy(
            socket->dataField.value.cast<std::string&>().data(),
            default_value_.data(),
            default_value_.size());
    }
}

void decl::Bool::update_default_value(NodeSocket* socket) const
{
    if (!socket->dataField.value) {
        socket->dataField.min = soft_min;
        socket->dataField.max = soft_max;
        socket->dataField.value =
            std::max(std::min(default_value_, soft_max), soft_min);
    }
}

#define BUILD_TYPE(NAME)                                             \
    NodeSocket* decl::NAME::build(NodeTree* ntree, Node* node) const \
    {                                                                \
        NodeSocket* socket = nodeAddSocket(                          \
            ntree,                                                   \
            node,                                                    \
            this->in_out,                                            \
            typeid(NAME).name(),                                     \
            this->identifier.c_str(),                                \
            this->name.c_str());                                     \
        update_default_value(socket);                                \
                                                                     \
        return socket;                                               \
    }

MACRO_MAP(BUILD_TYPE, ALL_SOCKET_TYPES)

NodeDeclarationBuilder::NodeDeclarationBuilder(NodeDeclaration& declaration)
    : declaration_(declaration)
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
