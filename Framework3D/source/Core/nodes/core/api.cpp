#include "nodes/core/api.hpp"

#include "nodes/core/node.hpp"
#include "nodes/core/node_exec_eager.hpp"
#include "nodes/core/node_link.hpp"
#include "nodes/core/node_tree.hpp"
#include "nodes/core/socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

entt::meta_ctx g_entt_ctx = entt::locator<entt::meta_ctx>::value_or();

entt::meta_ctx& get_entt_ctx()
{
    return g_entt_ctx;
}

SocketType get_socket_type(const char* t)
{
    if (std::string(t).empty()) {
        return SocketType();
    }
    return entt::resolve(get_entt_ctx(), entt::hashed_string{ t });
}

std::string get_type_name(SocketType type)
{
    if (!type) {
        return "";
    }
    return std::string(type.info().name());
}

template<>
SocketType get_socket_type<entt::meta_any>()
{
    return SocketType();
}

void unregister_cpp_type()
{
    entt::meta_reset(g_entt_ctx);
}

std::unique_ptr<NodeTree> create_node_tree(
    std::shared_ptr<NodeTreeDescriptor> descriptor)
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