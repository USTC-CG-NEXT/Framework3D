#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "Logger/Logger.h"
#include "entt/meta/factory.hpp"
#include "nodes/core/api.h"
#include "socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct NodeTreeExecutorDesc;
class NodeTreeDescriptor;
struct NodeTreeExecutor;

struct NodeLink;
class NodeTree;
class NodeDeclaration;

struct SocketID;
struct LinkId;
struct NodeId;

struct NodeTypeInfo;
class SocketDeclaration;
struct Node;
struct NodeSocket;

NODES_CORE_API entt::meta_ctx& get_entt_ctx();

// A wrapper for returnning the stripped string, instead of a string_view
template<typename T>
inline std::string type_name()
{
    return { entt::type_name<T>::value().data(),
             entt::type_name<T>::value().size() };
}

template<typename TYPE>
inline void register_cpp_type()
{
    entt::meta<TYPE>(get_entt_ctx()).type(entt::type_hash<TYPE>());
    if (!entt::hashed_string{ type_name<TYPE>().data() } ==
        entt::type_hash<TYPE>()) {
        log::error("register type failed: %s", type_name<TYPE>().data());
        std::cerr << "register type failed: " << type_name<TYPE>().data()
                  << std::endl;
    }
}

template<typename T>
SocketType get_socket_type()
{
    auto type =
        entt::resolve(get_entt_ctx(), entt::type_hash<std::decay_t<T>>());
    if (!type) {
        register_cpp_type<std::decay_t<T>>();
        log::info("register type: %s", type_name<T>().data());
        type =
            entt::resolve(get_entt_ctx(), entt::type_hash<std::decay_t<T>>());
        assert(type);
    }
    return type;
}

NODES_CORE_API SocketType get_socket_type(const char* t);
NODES_CORE_API std::string get_type_name(SocketType);

template<>
NODES_CORE_API SocketType get_socket_type<entt::meta_any>();

NODES_CORE_API void unregister_cpp_type();

NODES_CORE_API std::unique_ptr<NodeTree> create_node_tree(
    std::shared_ptr<NodeTreeDescriptor> descriptor);

NODES_CORE_API std::unique_ptr<NodeTreeExecutor> create_node_tree_executor(
    const NodeTreeExecutorDesc& desc);

namespace io {
NODES_CORE_API std::string serialize_node_tree(NodeTree* tree);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE