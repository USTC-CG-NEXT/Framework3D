#include "nodes/system/node_system.hpp"

#include <gtest/gtest.h>

#include "Logger/Logger.h"

using namespace USTC_CG;

class MyNodeSystem : public NodeSystem {
   public:
    bool load_configuration(const std::filesystem::path& config) override
    {
        return true;
    }

   private:
    std::shared_ptr<NodeTreeDescriptor> node_tree_descriptor() override
    {
        return std::make_shared<NodeTreeDescriptor>();
    }
};

TEST(NodeSystem, CreateSystem)
{
    MyNodeSystem system;
    system.init();
    ASSERT_TRUE(system.get_node_tree());
    // ASSERT_TRUE(system.get_node_tree_executor());
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

void print_tree_info(const NodeTree* tree)
{
    std::cout << "Nodes: " << tree->nodes.size() << std::endl;
    std::cout << "Links: " << tree->links.size() << std::endl;
    std::cout << "Sockets: " << tree->socket_count() << std::endl;

    std::cout << std::endl;
}

TEST(NodeSystem, DynamicSockets)
{
    auto dl_load_system = create_dynamic_loading_system();
    auto loaded = dl_load_system->load_configuration("test_nodes.json");
    ASSERT_TRUE(loaded);
    dl_load_system->init();

    auto tree = dl_load_system->get_node_tree();

    auto node = tree->add_node("add");

    ASSERT_TRUE(node);

    print_tree_info(tree);

    auto socket = node->group_add_socket(
        "input_group", type_name<int>().c_str(), "a", "a", PinKind::Input);

    print_tree_info(tree);

    node->group_remove_socket("input_group", "a", PinKind::Input);

    print_tree_info(tree);
}
