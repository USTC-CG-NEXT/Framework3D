#include "nodes/core/node_exec.hpp"

#include "nodes/core/node.hpp"
#include "nodes/core/node_exec_eager.hpp"
#include "nodes/core/node_link.hpp"
#include "nodes/core/socket.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

int ExeParams::get_input_index(const char* identifier) const
{
    return node_.find_socket_id(identifier, PinKind::Input);
}

std::vector<size_t> ExeParams::get_input_group_indices(
    const char* group_identifier) const
{
    return node_.find_socket_group_ids(group_identifier, PinKind::Input);
}
std::vector<size_t> ExeParams::get_output_group_indices(
    const char* group_identifier) const
{
    return node_.find_socket_group_ids(group_identifier, PinKind::Output);
}

int ExeParams::get_output_index(const char* identifier)
{
    return node_.find_socket_id(identifier, PinKind::Output);
}

std::unique_ptr<NodeTreeExecutor> create_executor(NodeTreeExecutorDesc& exec)
{
    switch (exec.policy) {
        case NodeTreeExecutorDesc::Policy::Eager:
            return std::make_unique<EagerNodeTreeExecutor>();
        case NodeTreeExecutorDesc::Policy::Lazy: return nullptr;
    }
    return nullptr;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
