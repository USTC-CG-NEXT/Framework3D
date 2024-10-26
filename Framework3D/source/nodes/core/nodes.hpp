#pragma once

#include "io/json.hpp"
#include "node.hpp"
#include "node_declare.hpp"
#include "node_socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

struct Node {
    NodeId ID;
    std::string ui_name;

    float Color[4];
    NodeType Type;

    unsigned Size[2];

    const NodeTypeInfo* typeinfo;

    bool REQUIRED = false;
    bool MISSING_INPUT = false;
    std::string execution_failed = {};

    std::function<void()> override_left_pane_info = nullptr;

    std::unique_ptr<NodeTree> subtree = nullptr;

    bool has_available_linked_inputs = false;
    bool has_available_linked_outputs = false;

    mutable nlohmann::json storage_info;
    mutable entt::meta_any storage;
    mutable entt::meta_any runtime_storage;

    explicit Node(NodeTree* node_tree, int id, const char* idname);

    Node(NodeTree* node_tree, const char* idname);

    void serialize(nlohmann::json& value);
    // During deserialization, we first deserialize all the sockets, then
    // according the info of the node, we record the information.
    void register_socket_to_node(NodeSocket* socket, PinKind in_out);

    NodeSocket* find_socket(const char* identifier, PinKind in_out) const;
    size_t find_socket_id(const char* identifier, PinKind in_out) const;

    [[nodiscard]] const std::vector<NodeSocket*>& get_inputs() const
    {
        return inputs;
    }

    [[nodiscard]] const std::vector<NodeSocket*>& get_outputs() const
    {
        return outputs;
    }

    bool valid()
    {
        return valid_;
    }

    void generate_socket_group_based_on_declaration(
        const SocketDeclaration& socket_declaration,
        const std::vector<NodeSocket*>& old_sockets,
        std::vector<NodeSocket*>& new_sockets);

    void remove_socket(NodeSocket* socket, PinKind kind);

    // For this deserialization, we assume there are some sockets already
    // present in the node tree.
    void deserialize(const nlohmann::json& node_json);

   private:
    void out_date_sockets(
        const std::vector<NodeSocket*>& olds,
        PinKind pin_kind);
    // refresh_node serves for this purpose - The node always complies with the
    // type description, while preserves the connection & id from the loaded
    // result. So we only outdate a limited set of the sockets.
    void refresh_node();
    bool pre_init_node(const char* idname);

    const NodeTypeInfo* nodeTypeFind(const char* idname);

    std::vector<NodeSocket*> inputs;
    std::vector<NodeSocket*> outputs;

    NodeTree* tree_;
    bool valid_ = false;
};

NodeSocket* nodeAddSocket(
    NodeTree* ntree,
    Node* node,
    PinKind pin_kind,
    const char* id_name,
    const char* identifier,
    const char* name);

NodeTypeInfo* nodeTypeFind(const char* idname);
// SocketType* socketTypeFind(const char* idname);

USTC_CG_NAMESPACE_CLOSE_SCOPE
