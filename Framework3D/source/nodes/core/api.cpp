#include "nodes/core/api.hpp"

#include "node_exec_eager.hpp"
#include "nodes/core/node.hpp"
#include "nodes/core/node_link.hpp"
#include "nodes/core/node_tree.hpp"
#include "nodes/core/socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

template<>
SocketType get_socket_type<entt::meta_any>()
{
    return SocketType();
}

std::unique_ptr<NodeTree> create_node_tree(const NodeTreeDescriptor& descriptor)
{
    return std::make_unique<NodeTree>(descriptor);
}

std::unique_ptr<NodeTreeExecutor> create_node_tree_executor(
    const NodeTreeExecutorDesc& desc)
{
    switch (desc.policy) {
        case NodeTreeExecutorDesc::Policy::Eager:
            return std::make_unique<EagerNodeTreeExecutor>();
        case NodeTreeExecutorDesc::Policy::Lazy: return nullptr;
    }
    return nullptr;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE