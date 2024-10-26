#include "node_exec_eager.hpp"

#include <set>

#include "USTC_CG.h"
#include "node_socket.hpp"
#include "entt/core/any.hpp"
#include "entt/meta/resolve.hpp"
#include "node_tree.hpp"
// #include "Utils/Functions/GenericPointer_.hpp"
//  #include "graph/node_exec_graph.h"
// #include "GCore/geom_node_global_params.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

ExeParams EagerNodeTreeExecutor::prepare_params(NodeTree* tree, Node* node)
{
    node->MISSING_INPUT = false;

    ExeParams params{ *node };
    for (auto&& input : node->get_inputs()) {
        entt::meta_any* input_ptr;

        if (input_states[index_cache[input]].is_forwarded) {
            // Is set by previous node
            input_ptr = &input_states[index_cache[input]].value;
        }
        else if (
            input->directly_linked_sockets.empty() && input->dataField.value) {
            auto type = input_states[index_cache[input]].value.type();
            auto value = input->dataField.value;
            // Has default value
            input_states[index_cache[input]].value = value;
            input_ptr = &input_states[index_cache[input]].value;
        }
        else {
            // Node not filled. Cannot run this node.
            input_ptr = &input_states[index_cache[input]].value;
            // input_ptr.type()->default_construct(input_ptr.get());

            node->MISSING_INPUT = true;
        }
        params.inputs_.push_back(input_ptr);
    }

    for (auto&& output : node->get_outputs()) {
        entt::meta_any* output_ptr = &output_states[index_cache[output]].value;
        params.outputs_.push_back(output_ptr);
    }

    return params;
}

bool EagerNodeTreeExecutor::execute_node(NodeTree* tree, Node* node)
{
    bool successfully_filled_data;
    if (try_fill_storage_to_node(node, successfully_filled_data))
        return successfully_filled_data;

    ExeParams params = prepare_params(tree, node);
    if (node->MISSING_INPUT) {
        return false;
    }
    auto typeinfo = node->typeinfo;
    try {
        typeinfo->node_execute(params);
        node->execution_failed = {};
    }
    catch (std::runtime_error(err)) {
        node->execution_failed = err.what();
        return false;
    }
    return true;
}

void EagerNodeTreeExecutor::forward_output_to_input(Node* node)
{
    for (auto&& output : node->get_outputs()) {
        if (output->directly_linked_sockets.empty()) {
            auto& output_state = output_states[index_cache[output]];
            assert(output_state.is_last_used == false);
            output_state.is_last_used = true;
        }
        else {
            int last_used_id = -1;

            bool need_to_keep_alive = false;

            for (int i = 0; i < output->directly_linked_sockets.size(); ++i) {
                auto directly_linked_input_socket =
                    output->directly_linked_sockets[i];

                if (std::string(directly_linked_input_socket->Node->typeinfo
                                    ->id_name) == "func_storage_in") {
                    need_to_keep_alive = true;
                }

                if (index_cache.find(directly_linked_input_socket) !=
                    index_cache.end()) {
                    if (directly_linked_input_socket->Node->REQUIRED) {
                        last_used_id = std::max(
                            last_used_id,
                            int(index_cache[directly_linked_input_socket]));
                    }

                    auto& input_state =
                        input_states[index_cache[directly_linked_input_socket]];
                    auto& output_state = output_states[index_cache[output]];
                    auto is_last_target =
                        i == output->directly_linked_sockets.size() - 1;

                    auto& value_to_forward = output_state.value;

                    if (!value_to_forward.type()) {
                        input_state.is_forwarded = true;
                    }

                    else if (
                        input_state.value.type() &&
                        input_state.value.type() != value_to_forward.type()) {
                        directly_linked_input_socket->Node->execution_failed =
                            "Type mismatch input";
                        input_state.is_forwarded = false;
                    }
                    else {
                        directly_linked_input_socket->Node
                            ->execution_failed = {};

                        if (is_last_target) {
                            input_state.value = std::move(value_to_forward);
                        }
                        else {
                            input_state.value = value_to_forward;
                        }
                        input_state.is_forwarded = true;
                    }
                }
            }

            if (need_to_keep_alive) {
                for (int i = 0; i < output->directly_linked_sockets.size();
                     ++i) {
                    auto directly_linked_input_socket =
                        output->directly_linked_sockets[i];

                    input_states[index_cache[directly_linked_input_socket]]
                        .keep_alive = true;
                }
            }

            if (last_used_id == -1) {
                output_states[index_cache[output]].is_last_used = true;
            }
            else {
                assert(input_states[last_used_id].is_last_used == false);

                input_states[last_used_id].is_last_used = true;
            }
        }
    }
}

void EagerNodeTreeExecutor::clear()
{
    input_states.clear();
    output_states.clear();
    index_cache.clear();
    nodes_to_execute.clear();
    nodes_to_execute_count = 0;
    input_of_nodes_to_execute.clear();
    output_of_nodes_to_execute.clear();
}

void EagerNodeTreeExecutor::compile(NodeTree* tree)
{
    if (tree->has_available_link_cycle) {
        return;
    }

    nodes_to_execute = tree->get_toposort_left_to_right();

    for (auto node : nodes_to_execute) {
        node->REQUIRED = false;
    }

    for (int i = nodes_to_execute.size() - 1; i >= 0; i--) {
        auto node = nodes_to_execute[i];

        if (node->typeinfo->ALWAYS_REQUIRED) {
            node->REQUIRED = true;
        }

        if (node->REQUIRED) {
            for (auto input : node->get_inputs()) {
                assert(input->directly_linked_sockets.size() <= 1);
                for (auto directly_linked_socket :
                     input->directly_linked_sockets) {
                    directly_linked_socket->Node->REQUIRED = true;
                }
            }
        }
    }

    auto split = std::stable_partition(
        nodes_to_execute.begin(), nodes_to_execute.end(), [](Node* node) {
            return node->REQUIRED;
        });

    // Now the nodes is split into two parts, and the topology sequence is
    // correct.

    nodes_to_execute_count = std::distance(nodes_to_execute.begin(), split);

    for (int i = 0; i < nodes_to_execute_count; ++i) {
        input_of_nodes_to_execute.insert(
            input_of_nodes_to_execute.end(),
            nodes_to_execute[i]->get_inputs().begin(),
            nodes_to_execute[i]->get_inputs().end());

        output_of_nodes_to_execute.insert(
            output_of_nodes_to_execute.end(),
            nodes_to_execute[i]->get_outputs().begin(),
            nodes_to_execute[i]->get_outputs().end());
    }
}

void EagerNodeTreeExecutor::prepare_memory()
{
    for (int i = 0; i < input_states.size(); ++i) {
        index_cache[input_of_nodes_to_execute[i]] = i;
        auto type = input_of_nodes_to_execute[i]->type_info;
        if (type) {
            input_states[i].value = type.construct();
        }
    }

    for (int i = 0; i < output_states.size(); ++i) {
        index_cache[output_of_nodes_to_execute[i]] = i;
        auto type = output_of_nodes_to_execute[i]->type_info;
        if (type) {
            output_states[i].value = type.construct();
        }
    }
}

void EagerNodeTreeExecutor::remove_storage(
    const std::set<std::string>::value_type& key)
{
    storage.erase(key);
}

void EagerNodeTreeExecutor::refresh_storage()
{
    std::set<std::string> refreshed;

    // After executing the tree, storage all the required info
    for (int i = 0; i < input_of_nodes_to_execute.size(); ++i) {
        auto socket = input_of_nodes_to_execute[i];
        if (!socket->type_info) {
            if (std::string(socket->Node->typeinfo->id_name) ==
                "func_storage_in") {
                auto node = socket->Node;
                entt::meta_any data;
                if (!socket->directly_linked_sockets.empty()) {
                    auto input = node->get_inputs()[0];
                    std::string name =
                        input->default_value_typed<std::string>();
                    if (storage.find(name) == storage.end()) {
                        data = socket->directly_linked_sockets[0]
                                   ->type_info.construct();
                        storage[name] = data;
                    }
                    refreshed.emplace(name);
                }
            }
        }
    }

    std::set<std::string> keysToDelete;
    for (auto&& value : storage) {
        if (!refreshed.contains(value.first)) {
            keysToDelete.emplace(value.first);
        }
    }
    for (auto& key : keysToDelete) {
        remove_storage(key);
    }
    refreshed.clear();
}

void EagerNodeTreeExecutor::try_storage()
{
    // After executing the tree, storage all the required info
    for (int i = 0; i < input_of_nodes_to_execute.size(); ++i) {
        auto socket = input_of_nodes_to_execute[i];
        if (!socket->type_info) {
            if (std::string(socket->Node->typeinfo->id_name) ==
                "func_storage_in") {
                auto node = socket->Node;
                entt::meta_any data;
                sync_node_to_external_storage(
                    input_of_nodes_to_execute[i], data);

                auto input = node->get_inputs()[0];
                std::string name = input->default_value_typed<std::string>();
                storage[name] = data;
            }
        }
    }
}

bool EagerNodeTreeExecutor::try_fill_storage_to_node(
    Node* node,
    bool& successfully_filled_data)
{
    // Identify the special storage node, and do a special execution here.

    if (node->REQUIRED) {  // requirement info is valid.
        if (std::string(node->typeinfo->id_name) == "func_storage_out") {
            auto input = node->get_inputs()[0];
            std::string name = input->default_value_typed<std::string>();
            if (storage.find(name) != storage.end()) {
                auto& storaged_value = storage.at(name);

                // Check all the connected input type

                for (auto input :
                     node->get_outputs()[0]->directly_linked_sockets) {
                    if (storaged_value.type() &&
                        storaged_value.type() !=
                            input_states[index_cache[input]].value.type()) {
                        node->execution_failed =
                            "Type Mismatch, filling default value.";
                        successfully_filled_data = false;
                        return true;
                    }
                }

                output_states[index_cache[node->get_outputs()[0]]].value =
                    storaged_value;

                node->execution_failed = {};
                successfully_filled_data = true;
                return true;
            }
            else {
                node->execution_failed =
                    "No cache can be found with name " + name + " (yet).";
                successfully_filled_data = false;
                return true;
            }
        }
    }
    return false;
}

EagerNodeTreeExecutor::~EagerNodeTreeExecutor()
{
    storage.clear();
}
//
// ExeParams EagerNodeTreeExecutorGeom::prepare_params(NodeTree* tree, Node*
// node)
//{
//    auto result = EagerNodeTreeExecutor::prepare_params(tree, node);
//    result.global_param = *global_param;
//
//    return result;
//}
//
// void EagerNodeTreeExecutorGeom::set_global_param(GeomNodeGlobalParams* param)
//{
//    this->global_param = param;
//}

void EagerNodeTreeExecutor::prepare_tree(NodeTree* tree)
{
    // auto gilState = PyGILState_Ensure();

    tree->ensure_topology_cache();
    clear();

    compile(tree);

    input_states.resize(input_of_nodes_to_execute.size());
    output_states.resize(output_of_nodes_to_execute.size());

    prepare_memory();

    refresh_storage();
    // PyGILState_Release(gilState);
}

void EagerNodeTreeExecutor::execute_tree(NodeTree* tree)
{
    // auto gilState = PyGILState_Ensure();

    for (int i = 0; i < nodes_to_execute_count; ++i) {
        auto node = nodes_to_execute[i];
        auto result = execute_node(tree, node);
        if (result) {
            forward_output_to_input(node);
        }
    }
    try_storage();

    // PyGILState_Release(gilState);
}

entt::meta_any* EagerNodeTreeExecutor::FindPtr(NodeSocket* socket)
{
    entt::meta_any* ptr;
    if (socket->in_out == PinKind::Input) {
        ptr = &input_states[index_cache[socket]].value;
    }
    else {
        ptr = &output_states[index_cache[socket]].value;
    }
    return ptr;
}

void EagerNodeTreeExecutor::sync_node_from_external_storage(
    NodeSocket* socket,
    const entt::meta_any& data)
{
    if (index_cache.find(socket) != index_cache.end()) {
        entt::meta_any* ptr = FindPtr(socket);
        *ptr = data;
    }
}

void EagerNodeTreeExecutor::sync_node_to_external_storage(
    NodeSocket* socket,
    entt::meta_any& data)
{
    if (index_cache.find(socket) != index_cache.end()) {
        const entt::meta_any* ptr = FindPtr(socket);
        data = *ptr;
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
