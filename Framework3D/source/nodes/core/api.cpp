#include "api.hpp"

#include "node.hpp"
#include "node_exec_eager.hpp"
#include "node_tree.hpp"
#include "socket.hpp"

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

std::unique_ptr<NodeTreeExecutor> create_node_tree_executor(ExecutorDesc& desc)
{
    switch (desc.policy) {
        case ExecutorDesc::Policy::Eager:
            return std::make_unique<EagerNodeTreeExecutor>();
        case ExecutorDesc::Policy::Lazy: return nullptr;
    }
    return nullptr;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE