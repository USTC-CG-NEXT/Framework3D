#pragma once

#include <map>
#include <memory>
#include <string>

#include "USTC_CG.h"
#include "entt/meta/factory.hpp"
#include "node.hpp"
#include "node_exec.hpp"
#include "socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class NodeTreeDescriptor;
struct NodeTreeExecutor;

class NodeLink;
class NodeTree;
class NodeDeclaration;

class SocketID;
class LinkId;
class NodeId;

struct NodeTypeInfo;
class SocketDeclaration;
struct Node;
struct NodeSocket;

template<typename T>
void register_cpp_type();

template<typename T>
SocketType get_socket_type()
{
    return entt::resolve(entt::type_hash<T>());
}

inline SocketType get_socket_type(const char* t)
{
    return entt::resolve(entt::hashed_string{ t });
}

template<>
SocketType get_socket_type<entt::meta_any>();

std::unique_ptr<NodeTree> create_node_tree(
    const NodeTreeDescriptor& descriptor);

std::unique_ptr<NodeTreeExecutor> create_node_tree_executor(ExecutorDesc& desc);

namespace io {
std::string serialize_node_tree(NodeTree* tree);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE

#include "internal/register_cpp_type.inl"
