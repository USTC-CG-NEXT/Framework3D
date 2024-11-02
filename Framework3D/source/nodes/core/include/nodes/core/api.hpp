#pragma once

#include <map>
#include <memory>
#include <string>

#include "entt/meta/factory.hpp"
#include "node.hpp"
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

template<typename TYPE>
inline void register_cpp_type()
{
    entt::meta<TYPE>(get_entt_ctx()).type(entt::type_hash<TYPE>());
    assert(
        entt::hashed_string{ typeid(TYPE).name() } == entt::type_hash<TYPE>());
}

template<typename T>
SocketType get_socket_type()
{
    return entt::resolve(get_entt_ctx(), entt::type_hash<T>());
}

NODES_CORE_API inline SocketType get_socket_type(const char* t);

template<>
NODES_CORE_API SocketType get_socket_type<entt::meta_any>();

NODES_CORE_API void unregister_cpp_type();

NODES_CORE_API std::unique_ptr<NodeTree> create_node_tree(
    const NodeTreeDescriptor& descriptor);

NODES_CORE_API std::unique_ptr<NodeTreeExecutor> create_node_tree_executor(
    const NodeTreeExecutorDesc& desc);

namespace io {
NODES_CORE_API std::string serialize_node_tree(NodeTree* tree);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE