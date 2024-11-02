#pragma once

#include "nodes/core/api.hpp"
#include "nodes/core/node_tree.hpp"
#include "nodes/system/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class NODES_SYSTEM_API NodeSystem {
   public:
    void init();
    virtual bool load_configuration(const std::filesystem::path& config) = 0;
    virtual ~NodeSystem();

    template<typename T1, typename... T>
    void register_cpp_types();

    [[nodiscard]] NodeTree* get_node_tree() const;
    [[nodiscard]] NodeTreeExecutor* get_node_tree_executor() const;

   protected:
    virtual NodeTreeDescriptor node_tree_descriptor() = 0;
    virtual NodeTreeExecutorDesc node_tree_executor_desc() = 0;
    std::unique_ptr<NodeTree> node_tree;
    std::unique_ptr<NodeTreeExecutor> node_tree_executor;
};

template<typename T1, typename... T>
void NodeSystem::register_cpp_types()
{
    register_cpp_type<T1>();
    if constexpr (sizeof...(T) > 0) {
        register_cpp_types<T...>();
    }
}

std::unique_ptr<NodeSystem> NODES_SYSTEM_API create_dynamic_loading_system();

USTC_CG_NAMESPACE_CLOSE_SCOPE