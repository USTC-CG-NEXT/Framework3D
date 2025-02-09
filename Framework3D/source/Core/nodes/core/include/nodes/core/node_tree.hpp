#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "api.hpp"
#include "node.hpp"
#include "node_exec.hpp"
#include "nodes/core/api.h"
#include "socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

// Multiple definitions of trees
class NODES_CORE_API NodeTreeDescriptor {
   public:
    NodeTreeDescriptor();
    ~NodeTreeDescriptor();

    NodeTreeDescriptor& register_node(const NodeTypeInfo& node_type);
    template<typename FROM, typename TO>
    NodeTreeDescriptor& register_conversion(
        const std::function<bool(const FROM&, TO&)>& conversion);
    NodeTreeDescriptor& register_conversion_name(
        const std::string& conversion_name);

    const NodeTypeInfo* get_node_type(const std::string& name) const;

    static std::string conversion_node_name(SocketType from, SocketType to);
    bool can_convert(SocketType from, SocketType to) const;

   private:
    friend class NodeWidget;
    std::map<std::string, NodeTypeInfo> node_registry;

    std::unordered_set<std::string> conversion_node_registry;
};

template<typename FROM, typename TO>
NodeTreeDescriptor& NodeTreeDescriptor::register_conversion(
    const std::function<bool(const FROM&, TO&)>& conversion)
{
    auto conversion_type_info = NodeTypeInfo(
        conversion_node_name(get_socket_type<FROM>(), get_socket_type<TO>())
            .c_str());

    conversion_type_info.ui_name = "invisible";

    conversion_type_info.set_declare_function([](NodeDeclarationBuilder& b) {
        b.add_input<FROM>("input");
        b.add_output<TO>("output");
    });

    conversion_type_info.set_execution_function([conversion](ExeParams params) {
        auto input = params.get_input<FROM>("input");
        TO output;
        conversion(input, output);
        params.set_output("output", std::move(output));
        return true;
    });
    conversion_type_info.INVISIBLE = true;

    conversion_node_registry.insert(conversion_type_info.id_name);
    node_registry[conversion_type_info.id_name] =
        std::move(conversion_type_info);

    return *this;
}

class NODES_CORE_API NodeTree {
   public:
    NodeTree(const NodeTreeDescriptor& descriptor);
    NodeTree(const NodeTree& other);

    NodeTree& operator=(const NodeTree& other);

    ~NodeTree();

    std::vector<std::unique_ptr<NodeLink>> links;
    std::vector<std::unique_ptr<Node>> nodes;
    bool has_available_link_cycle;

    unsigned input_socket_id(NodeSocket* socket);
    unsigned output_socket_id(NodeSocket* socket);

    std::vector<NodeSocket*> input_sockets;
    std::vector<NodeSocket*> output_sockets;

    [[nodiscard]] const std::vector<Node*>& get_toposort_right_to_left() const;

    [[nodiscard]] const std::vector<Node*>& get_toposort_left_to_right() const;

    // The left to right topology is holding the memory
    std::vector<Node*> toposort_right_to_left;
    std::vector<Node*> toposort_left_to_right;

    void clear();

    Node* find_node(NodeId id) const;

    NodeSocket* find_pin(SocketID id) const;

    NodeLink* find_link(LinkId id) const;

    bool is_pin_linked(SocketID id) const;

    Node* add_node(const char* str);

    Node* group_up(const std::vector<Node*>& nodes);

    void ungroup(Node* node)
    {
        throw std::runtime_error("Not implemented");
    }

    unsigned UniqueID();

    void update_socket_vectors_and_owner_node();
    void ensure_topology_cache();

    NodeLink* add_link(NodeSocket* fromsock, NodeSocket* tosock);

    NodeLink* add_link(SocketID startPinId, SocketID endPinId);

    void delete_link(
        LinkId linkId,
        bool refresh_topology = true,
        bool remove_from_group = true);
    void delete_link(
        NodeLink* link,
        bool refresh_topology = true,
        bool remove_from_group = true);

    void delete_node(Node* nodeId);
    void delete_node(NodeId nodeId);

    bool can_create_link(NodeSocket* node_socket, NodeSocket* node_socket1);
    static bool can_create_direct_link(
        NodeSocket* socket1,
        NodeSocket* socket2);
    bool can_create_convert_link(
        NodeSocket* node_socket,
        NodeSocket* node_socket1);
    friend struct Node;

    size_t socket_count() const;

    [[nodiscard]] const NodeTreeDescriptor& get_descriptor() const;

   private:
    // No one directly edits these sockets.
    std::vector<std::unique_ptr<NodeSocket>> sockets;
    const NodeTreeDescriptor descriptor_;

    void delete_socket(SocketID socketId);

    void update_directly_linked_links_and_sockets();

    // There is definitely better solution. However this is the most
    std::unordered_set<unsigned> used_ids;

    unsigned current_id = 1;

   public:
    std::string serialize(int indentation = -1) const;

    void deserialize(const std::string& str);

    void SetDirty(bool dirty = true);

    bool GetDirty();

   private:
    bool dirty_ = true;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE
