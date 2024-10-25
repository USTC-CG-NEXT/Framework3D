#include "api.hpp"

#include "node.hpp"
#include "node_socket.hpp"
#include "node_tree.hpp"
#include "socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace nodes {

std::shared_ptr<NodeTreeDescriptor> create_node_tree_descriptor()
{
    auto descriptor = std::make_shared<NodeTreeDescriptor>();

    return descriptor;
}

template<>
std::unique_ptr<SocketType> get_socket_type<entt::meta_any>()
{
    auto ptr = std::make_unique<SocketType>();
    return ptr;
}

std::unique_ptr<NodeTree> create_node_tree(
    std::shared_ptr<NodeTreeDescriptor> descriptor)
{
    return std::make_unique<NodeTree>(descriptor);
}
}  // namespace nodes

USTC_CG_NAMESPACE_CLOSE_SCOPE