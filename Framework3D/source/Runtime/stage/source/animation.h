#pragma once
#include <pxr/usd/usd/prim.h>
#include <stage/api.h>

#include "nodes/core/node_exec.hpp"
#include "nodes/system/node_system.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
namespace animation {

class WithDynamicLogic {
   public:
    virtual ~WithDynamicLogic() = default;
    virtual void update(float delta_time) const = 0;
};

class WithDynamicLogicPrim : public WithDynamicLogic {
   public:
    WithDynamicLogicPrim() { };
    WithDynamicLogicPrim(const pxr::UsdPrim& prim);

    WithDynamicLogicPrim(const WithDynamicLogicPrim& prim);
    WithDynamicLogicPrim& operator=(const WithDynamicLogicPrim& prim);

    void update(float delta_time) const override;
    static bool is_animatable(const pxr::UsdPrim& prim);

   private:
    mutable bool simulation_begun = false;

    pxr::UsdPrim prim;

    std::shared_ptr<NodeTree> node_tree;
    std::unique_ptr<NodeTreeExecutor> node_tree_executor;
    mutable std::string tree_desc_cache;

    static std::shared_ptr<NodeTreeDescriptor> node_tree_descriptor;
    static std::once_flag init_once;
};

}  // namespace animation

USTC_CG_NAMESPACE_CLOSE_SCOPE