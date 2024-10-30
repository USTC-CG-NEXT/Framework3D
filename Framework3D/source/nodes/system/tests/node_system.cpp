#include "nodes/system/node_system.hpp"

#include <gtest/gtest.h>

using namespace USTC_CG;

class MyNodeSystem : public NodeSystem {
   public:
    NodeTreeDescriptor node_tree_descriptor() override
    {
        return NodeTreeDescriptor();
    }

    NodeTreeExecutorDesc node_tree_executor_desc() override
    {
        return NodeTreeExecutorDesc();
    }
};

TEST(NodeSystem, CreateSystem)
{
    MyNodeSystem system;
    system.init();
    ASSERT_TRUE(system.get_node_tree());
    ASSERT_TRUE(system.get_node_tree_executor());
}