#pragma once

#include <set>

#include "hd_USTC_CG/render_global_payload.hpp"
#include "node_exec_eager_render.hpp"
#include "nodes/core/node_exec.hpp"
#include "nodes/core/node_exec_eager.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

class EagerNodeTreeExecutorRender : public EagerNodeTreeExecutor {
   protected:
    bool execute_node(NodeTree* tree, Node* node) override;

    void try_storage() override;
    void remove_storage(const std::set<std::string>::value_type& key) override;

   public:
    void finalize(NodeTree* tree) override;

    virtual void reset_allocator();

   private:
    ResourceAllocator& resource_allocator()
    {
        return global_payload.cast<RenderGlobalPayload&>().resource_allocator;
    }
};

USTC_CG_NAMESPACE_CLOSE_SCOPE