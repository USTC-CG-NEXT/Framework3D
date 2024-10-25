#pragma once

#include <map>
#include <memory>
#include <string>

#include "USTC_CG.h"
#include "entt/meta/factory.hpp"
#include "socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class NodeTreeDescriptor;
struct SocketType;
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

namespace nodes {

std::shared_ptr<NodeTreeDescriptor> create_node_tree_descriptor();

template<typename T>
void register_cpp_type();

template<typename T>
std::unique_ptr<SocketType> get_socket_type()
{
    return SocketType::get_socket_type(typeid(T).name());
}

template<>
std::unique_ptr<SocketType> get_socket_type<entt::meta_any>();

std::unique_ptr<NodeTree> create_node_tree(std::shared_ptr<NodeTreeDescriptor>);
std::unique_ptr<NodeTreeExecutor> create_node_tree_executor();

namespace io {
    std::string serialize_node_tree(NodeTree* tree);
}

}  // namespace nodes

USTC_CG_NAMESPACE_CLOSE_SCOPE

#include "internal/register_cpp_type.inl"
