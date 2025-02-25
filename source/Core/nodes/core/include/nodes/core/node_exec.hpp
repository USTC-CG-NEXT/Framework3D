#pragma once

#include <cassert>
#include <optional>
#include <vector>

#include "entt/meta/meta.hpp"
#include "entt/meta/resolve.hpp"
#include "node.hpp"
#include "nodes/core/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
struct NodeSocket;
struct Node;
class NodeTree;

struct NODES_CORE_API ExeParams {
    const Node& node_;

    explicit ExeParams(const Node& node, entt::meta_any& g_param)
        : node_(node),
          global_param(g_param)
    {
    }

    /**
     * Get the input value for the input socket with the given identifier.
     */
    template<typename T>
    T get_input(const char* identifier) const
    {
        if constexpr (std::is_same_v<T, entt::meta_any>) {
            const int index = this->get_input_index(identifier);
            return *inputs_[index];
        }
        else {
            const int index = this->get_input_index(identifier);
            return std::move(inputs_[index]->cast<T&&>());
        }
    }

    /**
     * Get the output value for the output socket with the given identifier.
     */

    template<typename T>
    std::vector<T> get_input_group(const char* group_identifier) const
    {
        static_assert(!std::is_same_v<T, entt::meta_any>);

        std::vector<size_t> indices =
            this->get_input_group_indices(group_identifier);

        std::vector<T> values;

        for (int index : indices) {
            values.push_back(inputs_[index]->cast<T>());
        }

        return values;
    }

    std::vector<entt::meta_any*> get_input_group(
        const char* group_identifier) const
    {
        std::vector<size_t> indices =
            this->get_input_group_indices(group_identifier);
        std::vector<entt::meta_any*> values;
        for (int index : indices) {
            values.push_back(inputs_[index]);
        }
        return values;
    }

    /**
     * Store the output value for the given socket identifier.
     */
    template<typename T>
    void set_output(const char* identifier, T&& value)
    {
        using DecayT = std::decay_t<T>;

        const int index = this->get_output_index(identifier);

        if (outputs_[index]->type()) {
            outputs_[index]->cast<DecayT&>() = std::forward<T>(value);
        }
        else {
            *outputs_[index] = std::forward<T>(value);
        }
    }

    template<typename T>
    T get_storage()
    {
        if (!node_.storage) {
            node_.storage = get_socket_type<T>().construct();
            if constexpr (std::decay_t<T>::has_storage) {
                if (!node_.storage_info.empty()) {
                    node_.storage.cast<T&>().deserialize(node_.storage_info);
                }
            }
        }

        return node_.storage.cast<T>();
    }

    template<typename T>
    void set_storage(T&& value)
    {
        node_.storage.cast<T&>() = value;
        if constexpr (std::decay_t<T>::has_storage) {
            node_.storage_info = value.serialize();
        }
    }

    template<typename T>
    T get_global_payload()
    {
        assert(global_param);
        return global_param.cast<T>();
    }

    NodeTreeExecutor* get_executor() const
    {
        return executor;
    }

    NodeTree* get_subtree() const
    {
        return subtree;
    }

    void set_output_group(
        const char* identifier,
        const std::vector<entt::meta_any>& outputs) const
    {
        const auto indices = get_output_group_indices(identifier);
        assert(indices.size() == outputs.size());
        for (size_t i = 0; i < indices.size(); ++i) {
            *outputs_[indices[i]] = outputs[i];
        }
    }

    void set_output_group(
        const char* identifier,
        std::vector<entt::meta_any>&& outputs) const
    {
        const auto indices = get_output_group_indices(identifier);
        assert(indices.size() == outputs.size());
        for (size_t i = 0; i < indices.size(); ++i) {
            *outputs_[indices[i]] = std::move(outputs[i]);
        }
    }

   private:
    int get_input_index(const char* identifier) const;
    std::vector<size_t> get_input_group_indices(
        const char* group_identifier) const;

    int get_output_index(const char* identifier);
    std::vector<size_t> get_output_group_indices(
        const char* group_identifier) const;

    friend class EagerNodeTreeExecutor;
    friend class EagerNodeTreeExecutorGeom;
    friend class EagerNodeTreeExecutorRender;

    template<typename T>
    friend T& force_get_output_to_execute(
        ExeParams& params,
        const char* identifier);

   private:
    entt::meta_any& global_param;
    std::vector<entt::meta_any*> inputs_;
    std::vector<entt::meta_any*> outputs_;

    // Subtree execution
    NodeTreeExecutor* executor;  // For node group execution
    NodeTree* subtree;
};

template<typename T>
T& force_get_output_to_execute(ExeParams& params, const char* identifier)
{
    if constexpr (std::is_same_v<T, entt::meta_any>) {
        const int index = params.get_output_index(identifier);
        return *params.outputs_[index];
    }
    else {
        const int index = params.get_output_index(identifier);
        return params.outputs_[index]->cast<T&>();
    }
}

// This executes a tree. The execution strategy is left to its children.
struct NODES_CORE_API NodeTreeExecutor {
   public:
    NodeTreeExecutor()
    {
    }

    virtual ~NodeTreeExecutor() = default;
    virtual void prepare_tree(
        NodeTree* tree,
        Node* required_node = nullptr) = 0;
    virtual void execute_tree(NodeTree* tree) = 0;
    virtual void finalize(NodeTree* tree)
    {
    }
    virtual void sync_node_from_external_storage(
        NodeSocket* socket,
        const entt::meta_any& data)
    {
    }

    virtual std::shared_ptr<NodeTreeExecutor> clone_empty() const = 0;

    virtual void sync_node_to_external_storage(
        NodeSocket* socket,
        entt::meta_any& data)
    {
    }
    void execute(NodeTree* tree, Node* required_node = nullptr)
    {
        prepare_tree(tree, required_node);
        execute_tree(tree);
    }

    template<typename T>
    T get_global_payload()
    {
        if (!global_payload) {
            global_payload = get_socket_type<T>().construct();
            if (!global_payload) {
                log::error("The global payload must be default constructable");
            }
        }
        return global_payload.cast<T>();
    }

   protected:
    entt::meta_any global_payload;
};

struct NodeTreeExecutorDesc {
    enum class Policy {
        Eager,
        Lazy,
    } policy = Policy::Eager;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE
