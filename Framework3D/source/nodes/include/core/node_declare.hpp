#pragma once

#include "USTC_CG.h"
#include "entt/meta/factory.hpp"
#include "socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

struct Node;
struct NodeSocket;
class NodeTree;
class NodeDeclarationBuilder;

/* Socket or panel declaration. */
class ItemDeclaration {
   public:
    virtual ~ItemDeclaration() = default;
};

using ItemDeclarationPtr = std::unique_ptr<ItemDeclaration>;

class SocketDeclaration : public ItemDeclaration {
   public:
    PinKind in_out;
    SocketType* type;
    std::string name;
    std::string identifier;

    virtual NodeSocket* build(NodeTree* ntree, Node* node) const = 0;

    virtual void update_default_value(NodeSocket* socket) const
    {
    }
};

/**
 * Describes a panel containing sockets or other panels.
 */
class PanelDeclaration : public ItemDeclaration { };

class BaseSocketDeclarationBuilder {
    int index_ = -1;

    NodeDeclarationBuilder* node_decl_builder_ = nullptr;
    friend class NodeDeclarationBuilder;
};

template<typename SocketDecl>
class SocketDeclarationBuilder : public BaseSocketDeclarationBuilder {
   protected:
    using Self = typename SocketDecl::Builder;
    static_assert(std::is_base_of_v<SocketDeclaration, SocketDecl>);
    std::unique_ptr<SocketDecl> decl_;

    friend class NodeDeclarationBuilder;
};

class NodeDeclaration {
   public:
    /* Combined list of socket and panel declarations.
     * This determines order of sockets in the UI and panel content. */
    std::vector<ItemDeclarationPtr> items;
    /* Note: inputs and outputs pointers are owned by the items list. */
    std::vector<SocketDeclaration*> inputs;
    std::vector<SocketDeclaration*> outputs;
};

namespace decl {
class Int;
class IntBuilder;

//template<typename T>
//class DeclClase : public SocketDeclaration {
//   public:
//    DeclClase()
//    {
//        type = nodes::get_socket_type<T>().get();
//    }
//
//    NodeSocket* build(NodeTree* ntree, Node* node) const override;
//
//    using Builder = DeclBuilder;
//};

}  // namespace decl
template<typename T>
struct SocketTrait {
    using Decl = T;
    using Builder = SocketDeclarationBuilder<Decl>;
};

template<>
struct SocketTrait<int> {
    using Decl = decl::Int;
    using Builder = decl::IntBuilder;
};

class NodeDeclarationBuilder {
   private:
    NodeDeclaration& declaration_;
    std::vector<std::unique_ptr<BaseSocketDeclarationBuilder>> socket_builders_;

   public:
    NodeDeclarationBuilder(NodeDeclaration& declaration);

    template<typename T>
    typename SocketTrait<T>::Builder& add_input(
        const char* name,
        const char* identifier = "");

    template<typename T>
    typename T::Builder& add_output(
        const char* name,
        const char* identifier = "");

    // The difference between storage and runtime storage is that storaged types
    // are expected to have the method of serialize() and deserialize().
    template<typename Data>
    void add_storage();

    template<typename Data>
    void add_runtime_storage();

   private:
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
typename T::Builder& NodeDeclarationBuilder::add_output(
    const char* name,
    const char* identifier)
{
    return add_socket<T>(name, "", identifier, PinKind::Output);
}

template<typename Data>
void NodeDeclarationBuilder::add_storage()
{
    entt::meta<Data>().type(entt::hashed_string{ typeid(Data).name() });
}

template<typename Data>
void NodeDeclarationBuilder::add_runtime_storage()
{
    entt::meta<Data>().type(entt::hashed_string{ typeid(Data).name() });
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

    socket_decl_builder->node_decl_builder_ = this;

    socket_decl_builder->decl_ = std::make_unique<Decl>();
    std::unique_ptr<Decl>& socket_decl = socket_decl_builder->decl_;
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
        assert(
            (std::find_if(
                 declaration_.inputs.begin(),
                 declaration_.inputs.end(),
                 [&](SocketDeclaration* socket) {
                     return socket->identifier == socket_decl->identifier;
                 }) == declaration_.inputs.end()));
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

USTC_CG_NAMESPACE_CLOSE_SCOPE
