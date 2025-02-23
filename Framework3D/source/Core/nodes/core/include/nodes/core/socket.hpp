#pragma once

#include <set>
#include <unordered_set>

#include "id.hpp"
#include "io/json.hpp"
#include "nodes/core/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct SocketGroup;
struct Node;
struct NodeLink;

enum class PinKind { Output, Input };

using SocketType = entt::meta_type;

struct NODES_CORE_API NodeSocket {
    char identifier[64];
    char ui_name[64];

    /** Only valid when #topology_cache_is_dirty is false. */
    std::vector<NodeLink*> directly_linked_links;
    std::vector<NodeSocket*> directly_linked_sockets;

    SocketID ID;
    Node* node;
    SocketGroup* socket_group = nullptr;
    std::string socket_group_identifier;

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
          node(nullptr),
          in_out(PinKind::Input)
    {
    }

    bool is_placeholder() const;

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

// Socket group is not a core component, rather a UI layer, giving the ability
// to dynamic modifying the node sockets.
struct NODES_CORE_API SocketGroup {
    Node* node;
    bool runtime_dynamic = false;
    PinKind kind;
    SocketType type_info;
    std::string identifier;
    NodeSocket* add_socket(
        const char* type_name,
        const char* identifier,
        const char* name,
        bool need_to_propagate_sync = true);

    NodeSocket* find_socket(const char* identifier)
    {
        auto it = std::find_if(
            sockets.begin(), sockets.end(), [identifier](NodeSocket* socket) {
                return strcmp(socket->identifier, identifier) == 0;
            });
        if (it == sockets.end()) {
            return nullptr;
        }
        return *it;
    }

    void add_sync_group(
        SocketGroup* group);  // TODO: later, the group inside and outside sync
                              // logic can be redo with this.

    void remove_socket(
        const char* identifier,
        bool need_to_propagate_sync = true);
    void remove_socket(NodeSocket* socket, bool need_to_propagate_sync = true);
    void serialize(nlohmann::json& value);
    void deserialize(const nlohmann::json& json);

   private:
    // Sometimes, we would like the some socket groups to always have the same
    // inputs or outputs
    std::set<SocketGroup*> synchronized_groups;

    std::vector<NodeSocket*> sockets;

    friend class Node;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE