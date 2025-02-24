#pragma once
#include <functional>
#include <memory>
#include <string>
#include <variant>

#include "api.hpp"
#include "entt/core/type_info.hpp"
#include "entt/meta/factory.hpp"
#include "entt/meta/meta.hpp"
#include "id.hpp"
#include "io/json.hpp"
#include "nodes/core/api.h"
#include "socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class SocketGroupDeclaration;
struct NodeTypeInfo;
class NodeDeclaration;
class SocketDeclaration;
enum class PinKind;
struct NodeSocket;
class NodeTree;
enum class NodeType;

struct ExeParams;
class Operator;
class NodeDeclarationBuilder;

using ExecFunction = std::function<bool(ExeParams params)>;
using NodeDeclareFunction =
    std::function<void(NodeDeclarationBuilder& builder)>;

namespace node {
std ::unique_ptr<NodeTypeInfo> make_node_type_info();

}  // namespace node

struct NODES_CORE_API Node {
    NodeId ID;
    std::string ui_name;

    float Color[4];

    unsigned Size[2];

    const NodeTypeInfo* typeinfo;  // Only holds the copy of the type_info

    bool REQUIRED = false;
    bool MISSING_INPUT = false;
    std::string execution_failed = {};

    std::function<void()> override_left_pane_info = nullptr;

    bool has_available_linked_inputs = false;
    bool has_available_linked_outputs = false;

    mutable nlohmann::json storage_info;
    mutable entt::meta_any storage;

    explicit Node(NodeTree* node_tree, int id, const char* idname);

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    SocketGroup* find_socket_group(
        const std::string& group_name,
        PinKind inout);

    Node(NodeTree* node_tree, const char* idname);
    virtual ~Node();

    virtual bool is_node_group();

    virtual void serialize(nlohmann::json& value);

    NodeSocket* get_output_socket(const char* identifier) const;
    NodeSocket* get_input_socket(const char* identifier) const;

    NodeSocket* find_socket(const char* identifier, PinKind in_out) const;
    size_t find_socket_id(const char* identifier, PinKind in_out) const;
    std::vector<size_t> find_socket_group_ids(
        const std::string& group_identifier,
        PinKind in_out) const;

    [[nodiscard]] const std::vector<NodeSocket*>& get_inputs() const;

    [[nodiscard]] const std::vector<NodeSocket*>& get_outputs() const;

    bool valid();

    void generate_sockets_based_on_declaration(
        const SocketDeclaration& socket_declaration,
        const std::vector<NodeSocket*>& old_sockets,
        std::vector<NodeSocket*>& new_sockets);

    void generate_socket_groups_socket(
        const SocketGroup* socket_group,
        const std::vector<NodeSocket*>& old_sockets,
        std::vector<NodeSocket*>& new_sockets);

    // For this deserialization, we assume there are some sockets already
    // present in the node tree.
    virtual void deserialize(const nlohmann::json& node_json);

    NodeSocket* add_socket(
        const char* type_name,
        const char* identifier,
        const char* name,
        PinKind in_out);

    virtual NodeSocket* group_add_socket(
        const std::string& socket_group_identifier,
        const char* type_name,
        const char* identifier,
        const char* name,
        PinKind in_out);

    virtual void group_remove_socket(
        const std::string& group_identifier,
        const char* identifier,
        PinKind in_out,
        bool is_recursive_call = false);

    // refresh_node serves for this purpose - The node always complies with the
    // type description, while preserves the connection & id from the loaded
    // result. So we only outdate a limited set of the sockets.
    void refresh_node();

    bool pre_init_node(const char* idname);
    Node* paired_node = nullptr;

   private:
    void remove_outdated_socket(NodeSocket* socket, PinKind kind);

    void out_date_sockets(
        const std::vector<NodeSocket*>& olds,
        PinKind pin_kind);

    const NodeTypeInfo* nodeTypeFind(const char* idname);

    bool valid_ = false;

   protected:
    // TODO: make inputs and outputs also managed by the nodes.
    std::vector<NodeSocket*> inputs;
    std::vector<NodeSocket*> outputs;

    // Each Node manages its own socket groups.
    std::vector<std::unique_ptr<SocketGroup>> socket_groups;
    NodeTree* tree_;

    // During deserialization, we first deserialize all the sockets, then
    // according the info of the node, we record the information.
    void register_socket_to_node(NodeSocket* socket, PinKind in_out);

    friend class NodeTree;
    friend class SocketGroup;
};

/**
 * struct NodeGroup
 * It is a Nodetree.
 * It can act as a single node.
 */
struct NodeGroup : public Node {
    NodeGroup(NodeTree* node_tree, const char* idname);

    NodeGroup(NodeTree* node_tree, int id, const char* idname);
    bool is_node_group() override;
    std::shared_ptr<NodeTree> sub_tree;

    void serialize(nlohmann::json& value) override;

    NodeSocket* group_add_socket(
        const std::string& socket_group_identifier,
        const char* type_name,
        const char* identifier,
        const char* name,
        PinKind in_out) override;
    void group_remove_socket(
        const std::string& group_identifier,
        const char* identifier,
        PinKind in_out,
        bool is_recursive_call = false) override;

    void deserialize(const nlohmann::json& node_json) override;

    friend class NodeTree;

    std::pair<NodeSocket*, NodeSocket*> node_group_add_input_socket(
        const char* type_name,
        const char* identifier,
        const char* name);

    std::pair<NodeSocket*, NodeSocket*> node_group_add_output_socket(
        const char* type_name,
        const char* identifier,
        const char* name);

   private:
    std::map<NodeSocket*, NodeSocket*> input_mapping_from_interface_to_internal;
    std::map<NodeSocket*, NodeSocket*>
        output_mapping_from_interface_to_internal;

    // Internal Node, Holding the input and output sockets.
    Node* group_in;
    Node* group_out;
};

NodeTypeInfo* nodeTypeFind(const char* idname);
// SocketType socketTypeFind(const char* idname);

/* Socket declaration. */
class ItemDeclaration {
   public:
    virtual ~ItemDeclaration() = default;
};

using ItemDeclarationPtr = std::shared_ptr<ItemDeclaration>;

class SocketDeclaration : public ItemDeclaration {
   public:
    PinKind in_out;
    SocketType type;
    std::string name;
    std::string identifier;

    virtual NodeSocket* build(NodeTree* ntree, Node* node) const = 0;

    virtual void update_default_value(NodeSocket* socket) const
    {
    }
};

class SocketGroupDeclaration : public ItemDeclaration {
   public:
    PinKind in_out;
    std::string identifier;
    bool runtime_dynamic = true;

    SocketType type;

    SocketGroup* build(NodeTree* ntree, Node* node) const;
};

class SocketGroupBuilder {
   public:
    SocketGroupBuilder(SocketGroupDeclaration* socket_group_declaration)
        : socket_group_declaration(socket_group_declaration)
    {
    }
    SocketGroupDeclaration* socket_group_declaration;
    SocketGroupBuilder& set_runtime_dynamic(bool runtime_dynamic = true)
    {
        socket_group_declaration->runtime_dynamic = runtime_dynamic;
        return *this;
    }

   private:
    friend class NodeDeclarationBuilder;
};

class BaseSocketDeclarationBuilder {
    int index_ = -1;

    friend class NodeDeclarationBuilder;
};

#include "socket_trait.inl"

template<
    typename T,
    bool HasMin = ValueTrait<T>::has_min,
    bool HasMax = ValueTrait<T>::has_max,
    bool HasDefault = ValueTrait<T>::has_default>
class Decl : public SocketDeclaration {
   public:
    using value_type = T;
    Decl()
    {
        type = get_socket_type<T>();
        // If type doesn't exist, throw
        if constexpr (!std::is_same_v<T, entt::meta_any>) {
            if (!type) {
                throw std::runtime_error("Type not found");
            }
        }
    }

    NodeSocket* build(NodeTree* ntree, Node* node) const override
    {
        NodeSocket* socket = node->add_socket(
            type_name<T>().data(),
            this->identifier.c_str(),
            this->name.c_str(),
            this->in_out);
        update_default_value(socket);

        return socket;
    }

    void update_default_value(NodeSocket* socket) const override
    {
        if (!socket->dataField.value) {
            if constexpr (HasMin) {
                socket->dataField.min = soft_min;
            }
            if constexpr (HasMax) {
                socket->dataField.max = soft_max;
            }
            if constexpr (HasMin && HasMax && HasDefault) {
                socket->dataField.value =
                    std::max(std::min(default_value, soft_max), soft_min);
            }
            else if constexpr (HasDefault) {
                socket->dataField.value = default_value;
            }
        }
    }

    // Only add the min field if the type has_min
    std::conditional_t<HasMin, T, std::monostate> soft_min;
    // Only add the max field if the type has_max
    std::conditional_t<HasMax, T, std::monostate> soft_max;
    // Only add the default field if the type has_default
    std::conditional_t<HasDefault, T, std::monostate> default_value;
};

template<typename SocketDecl>
class SocketDeclarationBuilder : public BaseSocketDeclarationBuilder {
   protected:
    static_assert(std::is_base_of_v<SocketDeclaration, SocketDecl>);
    SocketDecl* decl_;

    friend class NodeDeclarationBuilder;

   public:
    template<typename T = SocketDecl>
    SocketDeclarationBuilder& min(const typename T::value_type& min_value)
        requires ValueTrait<typename T::value_type>::has_min
    {
        decl_->soft_min = min_value;
        return *this;
    }

    template<typename T = SocketDecl>
    SocketDeclarationBuilder& max(const typename T::value_type& max_value)
        requires ValueTrait<typename T::value_type>::has_max
    {
        decl_->soft_max = max_value;
        return *this;
    }

    template<typename T = SocketDecl>
    SocketDeclarationBuilder& default_val(
        const typename T::value_type& default_val)
        requires ValueTrait<typename T::value_type>::has_default
    {
        decl_->default_value = default_val;
        return *this;
    }
};

template<typename T>
struct SocketTrait {
    using Decl = Decl<T>;
    using Builder = SocketDeclarationBuilder<Decl>;
};

class NodeDeclaration {
   public:
    std::vector<ItemDeclarationPtr> items;

    std::vector<SocketDeclaration*> inputs;
    std::vector<SocketDeclaration*> outputs;
    std::vector<SocketGroupDeclaration*> socket_group_decls;
};

class NodeDeclarationBuilder {
   private:
    NodeDeclaration& declaration_;
    std::vector<std::unique_ptr<BaseSocketDeclarationBuilder>> socket_builders_;

    std::vector<std::unique_ptr<SocketGroupBuilder>> group_builders_;

   public:
    NodeDeclarationBuilder(NodeDeclaration& declaration);

    template<typename T>
    typename SocketTrait<T>::Builder& add_input(
        const char* name,
        const char* identifier = "");

    template<typename T>
    typename SocketTrait<T>::Builder& add_output(
        const char* name,
        const char* identifier = "");

    template<typename T = entt::meta_any>
    SocketGroupBuilder& add_input_group(const char* identifier)
    {
        return add_group<T>(identifier, PinKind::Input);
    }

    SocketGroupBuilder& add_output_group(const char* identifier)
    {
        return add_group<entt::meta_any>(identifier, PinKind::Output);
    }

   private:
    template<typename T>
    SocketGroupBuilder& add_group(const char* identifier, PinKind in_out)
    {
        std::unique_ptr<SocketGroupDeclaration> group_decl =
            std::make_unique<SocketGroupDeclaration>();
        std::unique_ptr<SocketGroupBuilder> group_builder =
            std::make_unique<SocketGroupBuilder>(group_decl.get());

        group_decl->identifier = identifier;
        group_decl->in_out = in_out;
        group_decl->type = get_socket_type<T>();

        auto& group_builder_ref = *group_builder;

        declaration_.socket_group_decls.push_back(group_decl.get());
        group_builders_.push_back(std::move(group_builder));
        declaration_.items.push_back(std::move(group_decl));

        return group_builder_ref;
    }

    /* Note: in_out can be a combination of SOCK_IN and SOCK_OUT.
     * The generated socket declarations only have a single flag set. */
    template<typename T>
    typename SocketTrait<T>::Builder& add_socket(
        const char* name,
        const char* identifier_in,
        const char* identifier_out,
        PinKind in_out);
};

template<typename T>
typename SocketTrait<T>::Builder& NodeDeclarationBuilder::add_input(
    const char* name,
    const char* identifier)
{
    return add_socket<T>(name, identifier, "", PinKind::Input);
}

template<typename T>
typename SocketTrait<T>::Builder& NodeDeclarationBuilder::add_output(
    const char* name,
    const char* identifier)
{
    return add_socket<T>(name, "", identifier, PinKind::Output);
}

template<typename T>
typename SocketTrait<T>::Builder& NodeDeclarationBuilder::add_socket(
    const char* name,
    const char* identifier_in,
    const char* identifier_out,
    PinKind in_out)
{
    using Builder = typename SocketTrait<T>::Builder;
    using Decl = typename SocketTrait<T>::Decl;

    std::unique_ptr<Builder> socket_decl_builder = std::make_unique<Builder>();

    std::unique_ptr<Decl> socket_decl = std::make_unique<Decl>();
    socket_decl_builder->decl_ = socket_decl.get();
    socket_decl->name = name;
    socket_decl->in_out = in_out;
    socket_decl_builder->index_ = declaration_.inputs.size();

    if (in_out == PinKind::Input) {
        socket_decl->identifier = std::string(identifier_in);
        if (socket_decl->identifier.empty()) {
            socket_decl->identifier = name;
        }

        // Make sure there are no sockets in a same node with the same
        // identifier
        if (std::find_if(
                declaration_.inputs.begin(),
                declaration_.inputs.end(),
                [&](SocketDeclaration* socket) {
                    return socket->identifier == socket_decl->identifier;
                }) != declaration_.inputs.end()) {
            throw std::runtime_error(
                "Duplicate socket identifier found in inputs: " +
                socket_decl->identifier);
        }
        declaration_.inputs.push_back(socket_decl.get());
    }
    else {
        socket_decl->identifier = std::string(identifier_out);

        if (socket_decl->identifier.empty()) {
            socket_decl->identifier = name;
        }

        assert(
            std::find_if(
                declaration_.outputs.begin(),
                declaration_.outputs.end(),
                [&](SocketDeclaration* socket) {
                    return socket->identifier == socket_decl->identifier;
                }) == declaration_.outputs.end());

        declaration_.outputs.push_back(socket_decl.get());
    }
    declaration_.items.push_back(std::move(socket_decl));

    Builder& socket_decl_builder_ref = *socket_decl_builder;
    socket_builders_.push_back(std::move(socket_decl_builder));

    return socket_decl_builder_ref;
}

inline NodeDeclarationBuilder::NodeDeclarationBuilder(
    NodeDeclaration& declaration)
    : declaration_(declaration)
{
}

struct NODES_CORE_API NodeTypeInfo {
    NodeTypeInfo() = default;
    explicit NodeTypeInfo(const char* id_name);

    std::string id_name;
    std::string ui_name;

    NodeTypeInfo& set_ui_name(const std::string& ui_name);

    NodeTypeInfo& set_declare_function(
        const NodeDeclareFunction& decl_function);

    NodeTypeInfo& set_execution_function(const ExecFunction& exec_function);

    NodeTypeInfo& set_always_required(bool always_required);

    float color[4] = { 0.3, 0.5, 0.7, 1.0 };
    ExecFunction node_execute;

    bool ALWAYS_REQUIRED = false;
    bool INVISIBLE = false;

    NodeDeclaration static_declaration;

   private:
    NodeDeclareFunction declare;

    void reset_declaration();

    void build_node_declaration();
};
USTC_CG_NAMESPACE_CLOSE_SCOPE
