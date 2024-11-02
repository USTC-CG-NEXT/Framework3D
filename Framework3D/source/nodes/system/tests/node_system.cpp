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

    bool load_configuration(const std::filesystem::path& config) override
    {
        return true;
    }
};

TEST(NodeSystem, CreateSystem)
{
    MyNodeSystem system;
    system.init();
    ASSERT_TRUE(system.get_node_tree());
    ASSERT_TRUE(system.get_node_tree_executor());
}

TEST(NodeSystem, LoadDyLib)
{
    auto dl_load_system = create_dynamic_loading_system();

    auto loaded = dl_load_system->load_configuration("test_nodes.json");

    ASSERT_TRUE(loaded);
    dl_load_system->init();
}

TEST(NodeSystem, LoadDyLibExecution)
{
    auto dl_load_system = create_dynamic_loading_system();

    auto loaded = dl_load_system->load_configuration("test_nodes.json");

    ASSERT_TRUE(loaded);
    dl_load_system->init();
}
