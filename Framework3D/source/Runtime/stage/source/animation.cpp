#include "animation.h"

#include "../../../Editor/geometry/include/GCore/geom_payload.hpp"
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/usdGeom/xform.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
namespace animation {

std::once_flag WithDynamicLogicPrim::init_once;
std::shared_ptr<NodeTreeDescriptor> WithDynamicLogicPrim::node_tree_descriptor =
    nullptr;

WithDynamicLogicPrim::WithDynamicLogicPrim(const pxr::UsdPrim& prim)
    : prim(prim)
{
    std::call_once(init_once, [&] {
        // Only using this to initialize the node descriptor
        std::shared_ptr<NodeSystem> node_system =
            create_dynamic_loading_system();

        auto loaded = node_system->load_configuration("geometry_nodes.json");
        loaded = node_system->load_configuration("basic_nodes.json");
        node_tree_descriptor = node_system->node_tree_descriptor();
    });

    node_tree = std::make_shared<NodeTree>(node_tree_descriptor);
    NodeTreeExecutorDesc executor_desc;
    executor_desc.policy = NodeTreeExecutorDesc::Policy::Eager;
    executor_desc.is_simulation = true;
    node_tree_executor = create_node_tree_executor(executor_desc);
}

WithDynamicLogicPrim::WithDynamicLogicPrim(const WithDynamicLogicPrim& prim)
{
    this->prim = prim.prim;
    this->node_tree = prim.node_tree;
    NodeTreeExecutorDesc executor_desc;

    executor_desc.policy = NodeTreeExecutorDesc::Policy::Eager;
    executor_desc.is_simulation = true;
    this->node_tree_executor = create_node_tree_executor(executor_desc);
}

WithDynamicLogicPrim& WithDynamicLogicPrim::operator=(
    const WithDynamicLogicPrim& prim)
{
    this->prim = prim.prim;
    this->node_tree = prim.node_tree;
    NodeTreeExecutorDesc executor_desc;

    executor_desc.policy = NodeTreeExecutorDesc::Policy::Eager;
    executor_desc.is_simulation = true;
    this->node_tree_executor = create_node_tree_executor(executor_desc);
    return *this;
}

void WithDynamicLogicPrim::update(float delta_time) const
{
    assert(node_tree);
    assert(node_tree_executor);

    auto& payload = node_tree_executor->get_global_payload<GeomPayload&>();
    payload.delta_time = delta_time;

    node_tree_executor->execute(node_tree.get());
}

// Check whethe r important attributes have time samples
bool WithDynamicLogicPrim::is_animatable(const pxr::UsdPrim& prim)
{
    auto animatable = prim.GetAttribute(pxr::TfToken("Animatable"));

    if (!animatable) {
        return false;
    }
    bool is_animatable = false;
    animatable.Get(&is_animatable);
    return is_animatable;
}
}  // namespace animation

USTC_CG_NAMESPACE_CLOSE_SCOPE