#pragma once

#include "nodes/core/node_tree.hpp"
#include "nodes/system/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class NODES_SYSTEM_API NodeSystem {
   public:
    void init();
    virtual ~NodeSystem();

    [[nodiscard]] NodeTree* get_node_tree() const;
    [[nodiscard]] NodeTreeExecutor* get_node_tree_executor() const;

   private:
    virtual NodeTreeDescriptor node_tree_descriptor() = 0;
    virtual NodeTreeExecutorDesc node_tree_executor_desc() = 0;
    std::unique_ptr<NodeTree> node_tree;
    std::unique_ptr<NodeTreeExecutor> node_tree_executor;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE