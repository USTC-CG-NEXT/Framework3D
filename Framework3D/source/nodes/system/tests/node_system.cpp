#include "nodes/system/node_system.hpp"

#include <gtest/gtest.h>

using namespace USTC_CG;

class MyNodeSystem : public NodeSystem {
   public:
    bool load_configuration(const std::filesystem::path& config) override
    {
        return true;
    }

   private:
    NodeTreeDescriptor node_tree_descriptor() override
    {
        return NodeTreeDescriptor();
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
    dl_load_system->register_cpp_types<int>();

    auto loaded = dl_load_system->load_configuration("test_nodes.json");

    ASSERT_TRUE(loaded);
    dl_load_system->init();
}

TEST(NodeSystem, LoadDyLibExecution)
{
    auto dl_load_system = create_dynamic_loading_system();
    dl_load_system->register_cpp_types<int>();

    auto loaded = dl_load_system->load_configuration("test_nodes.json");

    ASSERT_TRUE(loaded);
    dl_load_system->init();
}
