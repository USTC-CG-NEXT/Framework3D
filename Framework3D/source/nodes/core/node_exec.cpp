#include "nodes/core/node_exec.hpp"

#include "nodes/core/node.hpp"
#include "node_exec_eager.hpp"


USTC_CG_NAMESPACE_OPEN_SCOPE
int ExeParams::get_input_index(const char* identifier) const
{
    return node_.find_socket_id(identifier, PinKind::Input);
}

int ExeParams::get_output_index(const char* identifier)
{
    return node_.find_socket_id(identifier, PinKind::Output);
}

std::unique_ptr<NodeTreeExecutor> create_executor(ExecutorDesc& exec)
{
    switch (exec.policy) {
        case ExecutorDesc::Policy::Eager:
            return std::make_unique<EagerNodeTreeExecutor>();
        case ExecutorDesc::Policy::Lazy: return nullptr;
    }
    return nullptr;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
