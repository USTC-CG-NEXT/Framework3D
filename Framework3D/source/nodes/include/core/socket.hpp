#pragma once

#include <string>
#include <unordered_set>

#include "USTC_CG.h"
#include "id.hpp"
#include "io/json.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct Node;
struct NodeLink;

enum class PinKind { Output, Input, Storage };

using SocketType = entt::meta_type;

struct NodeSocket {
    char identifier[64];
    char ui_name[64];

    /** Only valid when #topology_cache_is_dirty is false. */
    std::vector<NodeLink*> directly_linked_links;
    std::vector<NodeSocket*> directly_linked_sockets;

    SocketID ID;
    Node* Node;

    SocketType type_info;
    PinKind in_out;

    // This is for simple data fields in the node graph.
    struct bNodeSocketValue {
        entt::meta_any value;
        entt::meta_any min;
        entt::meta_any max;
    } dataField;

    explicit NodeSocket(int id = 0)
        : identifier{},
          ui_name{},
          ID(id),
          Node(nullptr),
          in_out(PinKind::Input)
    {
    }

    void Serialize(nlohmann::json& value);
    void DeserializeInfo(nlohmann::json& value);
    void DeserializeValue(const nlohmann::json& value);

    /** Utility to access the value of the socket. */
    template<typename T>
    T default_value_typed();
    template<typename T>
    const T& default_value_typed() const;

    friend bool operator==(const NodeSocket& lhs, const NodeSocket& rhs)
    {
        return strcmp(lhs.identifier, rhs.identifier) == 0 &&
               lhs.dataField.value == rhs.dataField.value;
    }

    friend bool operator!=(const NodeSocket& lhs, const NodeSocket& rhs)
    {
        return !(lhs == rhs);
    }

    ~NodeSocket()
    {
    }
};

template<typename T>
T NodeSocket::default_value_typed()
{
    return dataField.value.cast<T>();
}

template<typename T>
const T& NodeSocket::default_value_typed() const
{
    return dataField.value.cast<const T&>();
}

struct SocketGroup {
    std::vector<NodeSocket*> sockets;
    bool runtime_dynamic = false;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE