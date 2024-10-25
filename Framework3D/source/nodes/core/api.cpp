#include "api.hpp"

#include "node_socket.hpp"
#include "node_tree.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
std::shared_ptr<NodeTreeDescriptor> nodes::create_node_tree_descriptor()
{
    auto descriptor = std::make_shared<NodeTreeDescriptor>();

    return descriptor;
}

template<>
std::unique_ptr<SocketTypeInfo> nodes::get_socket_type_info<entt::meta_any>()
{
    auto ptr = std::make_unique<SocketTypeInfo>();
    ptr->type_name = typeid(entt::meta_any).name();
    return ptr;
}

std::unique_ptr<SocketTypeInfo> nodes::get_socket_type_info(const char* type)
{
    std::unique_ptr<SocketTypeInfo> socket_type_info =
        std::make_unique<SocketTypeInfo>(type);
    return socket_type_info;
}

std::unique_ptr<NodeTree> nodes::create_node_tree(
    std::shared_ptr<NodeTreeDescriptor> descriptor)
{
    return std::make_unique<NodeTree>(descriptor);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE