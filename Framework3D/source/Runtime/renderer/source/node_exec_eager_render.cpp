#include "node_exec_eager_render.hpp"

#include <set>

#include "hd_USTC_CG/render_global_payload.hpp"
#include "nodes/core/node_exec.hpp"
#include "nodes/core/node_exec_eager.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

bool EagerNodeTreeExecutorRender::execute_node(NodeTree* tree, Node* node)
{
    if (EagerNodeTreeExecutor::execute_node(tree, node)) {
        for (auto&& input : node->get_inputs()) {
            auto& input_state = input_states[index_cache[input]];
            if (!node->typeinfo->ALWAYS_REQUIRED && input_state.is_last_used) {
                if (input_state.value && !input_state.keep_alive)
                    resource_allocator().destroy(input_state.value);
                input_state.is_last_used = false;
            }
        }
        return true;
    }
    for (auto&& output : node->get_outputs()) {
        {
            if (output_states[index_cache[output]].value)
                resource_allocator().destroy(
                    output_states[index_cache[output]].value);
        }
    }
    return false;
}

void EagerNodeTreeExecutorRender::try_storage()
{
    for (auto&& value : storage) {
        resource_allocator().destroy(value.second);
    }
    EagerNodeTreeExecutor::try_storage();
}

void EagerNodeTreeExecutorRender::remove_storage(
    const std::set<std::string>::value_type& key)
{
    resource_allocator().destroy(storage[key]);
    EagerNodeTreeExecutor::remove_storage(key);
}

void EagerNodeTreeExecutorRender::finalize(NodeTree* tree)
{
    for (int i = 0; i < input_states.size(); ++i) {
        if (input_states[i].is_last_used && !input_states[i].keep_alive) {
            resource_allocator().destroy(input_states[i].value);
            input_states[i].is_last_used = false;
        }
    }

    for (int i = 0; i < output_states.size(); ++i) {
        if (output_states[i].is_last_used) {
            resource_allocator().destroy(output_states[i].value);
            output_states[i].is_last_used = false;
        }
    }
}

void EagerNodeTreeExecutorRender::reset_allocator()
{
    for (auto&& value : storage) {
        resource_allocator().destroy(value.second);
    }
    resource_allocator().terminate();
    storage.clear();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
